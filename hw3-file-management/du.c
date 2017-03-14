#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

//const int PATH_MAX_LENGTH = 4096;
//const int VISITED_INODE_SIZE = 4096;
#define PATH_MAX_LENGTH 4096
#define VISITED_INODE_SIZE 4096

int isVisitedInode(ino_t inode, ino_t *visitedInodes, int length);
int getDiskSpaceFiles(char *directoryPath, DIR *dirp, char *fullPath, char *childPath, ino_t **visitedInodes, int *lastUsedInodeIndex, int visitedInodesLength);
int getDiskSpaceDirectories(char *directoryPath, DIR *dirp, char *fullPath, char *childPath, ino_t **visitedInodes, int *lastUsedInodeIndex, int visitedInodesLength);
int getDiskSpace(char *directoryPath, ino_t **visitedInodes, int *lastUsedInodeIndex, int visitedInodesLength);


int main(int argc, char **argv) {
	char *directoryPath = ".";
	ino_t *visitedInodes;
	if ((visitedInodes = calloc(VISITED_INODE_SIZE, sizeof(ino_t))) == NULL) {
		perror("Cannot allocate memory");
		exit(1);
	}
	if (argc > 1) {
		directoryPath = argv[1];
	}
	int lastUsedInodeIndex = 0;
	int size = getDiskSpace(directoryPath, &visitedInodes, &lastUsedInodeIndex, VISITED_INODE_SIZE);
	printf("%d\t%s\n", size, directoryPath);
	free(visitedInodes);
	return 0;
}

int getDiskSpace(char *directoryPath, ino_t **visitedInodes, int *lastUsedInodeIndex, int visitedInodesLength) {
	/*
	Calculates the disk usage for the specified directory
	Outputs the disk usage for child directories

	directoryPath - path to parent directory to get disk space of
	visitedInodes - pointer to array of inodes that we saw
	lastUsedInodeIndex - points to int holding last used inode in visitedInodes
	visitedInodesLength - size of visitedInodes
	*/
	DIR *dirp;
	if ((dirp = opendir(directoryPath)) == NULL) {
		perror("Cannot open directory");
		exit(1);
	}
	char *fullPath;
	if ((fullPath = (char *)calloc(PATH_MAX_LENGTH, sizeof(char))) == NULL) {
		perror("Cannot allocate memory");
		exit(1);
	}

	int pathLength = strlen(directoryPath);
	strncpy(fullPath, directoryPath, pathLength);
	fullPath[pathLength] = '/';
	char *childPath = fullPath + pathLength + 1;
	long total = 0;
	int directoryBeginning = telldir(dirp);
	total += getDiskSpaceFiles(directoryPath, dirp, fullPath, childPath, visitedInodes, lastUsedInodeIndex, visitedInodesLength);

	seekdir(dirp, directoryBeginning);
	total += getDiskSpaceDirectories(directoryPath, dirp, fullPath, childPath, visitedInodes, lastUsedInodeIndex, visitedInodesLength);

	closedir(dirp);
	free(fullPath);
	return total;
}

int getDiskSpaceFiles(char *directoryPath, DIR *dirp, char *fullPath, char *childPath, ino_t **visitedInodes, int *lastUsedInodeIndex, int visitedInodesLength) {
	/*
	Checks the disk usage for files in a directory
	Doesn't count soft links and doesn't double count hard links

	fullPath - used to store the full path to the child element
	childPath - used directly to modify the path to point to the child element
	- must point to an entry in fullPath
	visitedInodes - pointer to array of i-nodes that were seen
	- ensures we don't double count hard links
	lastUsedInodeIndex - points to a int holding the last index used in visitedInodes
	visitedInodesLength - size of visitedInodes
	*/
	int total = 0;
	for (struct dirent *directoryEntry = readdir(dirp); directoryEntry != NULL; directoryEntry = readdir(dirp)) {
		int childNameLength = strlen(directoryEntry->d_name);
		strncpy(childPath, directoryEntry->d_name, childNameLength);
		childPath[childNameLength] = '\0';

		struct stat dirstat;
		if (lstat(fullPath, &dirstat) != 0) {
			perror("Cannot use stat");
			exit(1);
		}
		if (S_ISREG(dirstat.st_mode)) {
			unsigned size = (int)dirstat.st_blocks / 2;

			if (dirstat.st_nlink == 1) {
				total += size;
			} else if (isVisitedInode(dirstat.st_ino, *visitedInodes, visitedInodesLength) == 0) {
				total += size;
				if (*lastUsedInodeIndex == visitedInodesLength) {
					perror("Did not account for all inodes");
					exit(1);
				}

				*(*visitedInodes + *lastUsedInodeIndex) = dirstat.st_ino;
				++(*lastUsedInodeIndex);
			}
		}
	}
	return total;
}

int getDiskSpaceDirectories(char *directoryPath, DIR *dirp, char *fullPath, char *childPath, ino_t **visitedInodes, int *lastUsedInodeIndex, int visitedInodesLength) {
	/*
	Checks the disk usage for a particular directory's subdirectories.
	It will recursively find the disk usage of any subdirectories.

	fullPath - used to store the full path to the child element
	childPath - used directly to modify the path to point to the child element
	- must point to an entry in fullPath
	visitedInodes - pointer to array of i-nodes that were seen
	- ensures we don't double count hard links
	lastUsedInodeIndex - points to a int holding the last index used in visitedInodes
	visitedInodes
	visitedInodesLength - size of visitedInodes
	*/
	int total = 0;
	for (struct dirent *directoryEntry = readdir(dirp); directoryEntry != NULL; directoryEntry = readdir(dirp)) {
		if (strcmp("..", directoryEntry->d_name) != 0) {
			int childNameLength = strlen(directoryEntry->d_name);
			strncpy(childPath, directoryEntry->d_name, childNameLength);
			childPath[childNameLength] = '\0';

			struct stat dirstat;
			if (lstat(fullPath, &dirstat) != 0) {
				perror("Cannot use stat");
				exit(1);
			}
			unsigned size = (int)dirstat.st_blocks / 2;
			if ((dirstat.st_mode & S_IFMT) == S_IFDIR) {
				if (strcmp(".", directoryEntry->d_name) != 0) {
					size = getDiskSpace(fullPath, visitedInodes, lastUsedInodeIndex, visitedInodesLength);
					printf("%d\t%s\n", size, fullPath);
				}
				total += size;
			}
		}
	}
	return total;
}

int isVisitedInode(ino_t inode, ino_t *visitedInodes, int length) {
	/*
	Checks to see if the specified inode has been visited
	*/
	for (int i = 0; i < length; ++i) {
		if (*(visitedInodes + i) == inode) {
			return 1;
		}
	}
	return 0;
}
