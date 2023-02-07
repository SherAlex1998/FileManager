#include "os_file.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>

#pragma region Consts
const int countMemoryItemsSize = 256;
#pragma endregion

using namespace std;

#pragma region Structs
struct Folder;

struct File;

struct Folder
{
	char* name;
	Folder* parent;
	Folder** sons;
	File** files;
	int sonsCount;
	int filesCount;
};

struct File
{
	char* name;
	Folder* parent;
	int size;
};

struct ParsedPath
{
	char** path;
	int tokenCount;
};
#pragma endregion

Folder* rootFolder;
Folder* currentFolder;

int availableDiskSize;

#pragma region AssistFunctions
int Length(const char* str)
{
	int len = 0;
	while (*str != '\0')
	{
		str++;
		len++;
	}
	return len;
}

void PrintParsedPath(ParsedPath* parsedPath)
{
	for (int i = 0; i < parsedPath->tokenCount; i++)
	{
		int len = Length(parsedPath->path[i]);
		for (int j = 0; j < len; j++)
		{
			cout << parsedPath->path[i][j];
		}
		//cout << endl;
	}
	cout << endl;
}

ParsedPath* GetParsedPath(const char* path)
{
	ParsedPath* parsedPath = (ParsedPath*)malloc(sizeof(ParsedPath));
	parsedPath->path = (char**)malloc(countMemoryItemsSize * sizeof(char*));
	if (parsedPath->path)
	{
		for (int i = 0; i < countMemoryItemsSize; i++)
			parsedPath->path[i] = (char*)malloc(countMemoryItemsSize * sizeof(char));
	}
	else
	{
		return nullptr;
	}
	int length = Length(path);
	int parsedPathIndex = 0;
	if (path[0] == '/')
	{
		if (parsedPath->path[0]) strcpy(parsedPath->path[0], "/\0"); else return nullptr;
		parsedPathIndex++;
	}
	int symbolIndex = 0;
	for (int i = parsedPathIndex; i < length; i++)
	{
		if (path[i] == '/')
		{
			parsedPath->path[parsedPathIndex][symbolIndex] = '\0';
			parsedPathIndex++;
			symbolIndex = 0;
			continue;
		}
		char c = path[i];
		parsedPath->path[parsedPathIndex][symbolIndex] = path[i];
		symbolIndex++;
	}
	parsedPath->path[parsedPathIndex][symbolIndex] = '\0';
	parsedPath->tokenCount = ++parsedPathIndex;
	return parsedPath;
}

Folder* CreateFolder(const char* name, Folder* parent)
{
	Folder* newFolder = (Folder*)malloc(sizeof(Folder));
	if (newFolder->parent) newFolder->parent = parent; else return nullptr;
	newFolder->name = (char*)malloc(countMemoryItemsSize * sizeof(char));
	if (newFolder->name) strcpy(newFolder->name, name); else return nullptr;
	newFolder->sons = (Folder**)malloc(countMemoryItemsSize * sizeof(Folder*));
	newFolder->files = (File**)malloc(countMemoryItemsSize * sizeof(File*));
	newFolder->sonsCount = 0;
	newFolder->filesCount = 0;
	return newFolder;
}

File* CreateFile(const char* name, int size, Folder* parent)
{
	File* newFile = (File*)malloc(sizeof(File));
	if (newFile->parent) newFile->parent = parent; else return nullptr;
	newFile->name = (char*)malloc(countMemoryItemsSize * sizeof(char));
	if (newFile->name) strcpy(newFile->name, name); else return nullptr;
	newFile->size = size;
	return newFile;
}

void PrintIndent(int indent)
{
	indent *= 2;
	for (int i = 0; i < indent; i++) cout << ' ';
}

void PrintFolderStructureRec(Folder* folder, int depth)
{
	if (folder == nullptr) return;
	PrintIndent(depth);
	cout << folder->name << ":" << endl;
	for (int i = 0; i < folder->sonsCount; i++)
	{
		PrintFolderStructureRec(folder->sons[i], ++depth);
	}
	for (int i = 0; i < folder->filesCount; i++)
	{
		PrintIndent(depth);
		cout << folder->files[i]->name << endl;
	}
}

void PrintFolderStructure(Folder* folder)
{
	PrintFolderStructureRec(folder, 0);
}

Folder* FindFolder(ParsedPath* parsedPath)
{
	//ParsedPath* parsedPath = GetParsedPath(path);
	if (parsedPath->tokenCount == 2) // Crutch for "./" and "/" when Parsed Path will containe 2 tokens, whene second token is empty
	{
		if (parsedPath->path[1][0] == '\0')
		{
			return rootFolder;
		}
	}
	Folder* desiredFolder;
	int isAbsplute = 0;
	if (strcmp(parsedPath->path[0], "/") == 0)
	{
		desiredFolder = rootFolder;
		isAbsplute = 1;
	}
	else
	{
		desiredFolder = currentFolder;
	}
	for (int i = isAbsplute; i < parsedPath->tokenCount; i++)
	{
		if (Length(parsedPath->path[i]) == 0)
		{
			//cout << "fail" << endl;
			return nullptr;
		}
		if (strcmp(parsedPath->path[i], "..") == 0)
		{
			if (desiredFolder->parent == nullptr) return nullptr;
			desiredFolder = desiredFolder->parent;
		}
		else if (strcmp(parsedPath->path[i], ".") == 0)
		{
			continue;
		}
		else
		{
			int isDesiredFolderFinded = 0;
			for (int j = 0; j < desiredFolder->sonsCount; j++)
			{
				if (strcmp(parsedPath->path[i], desiredFolder->sons[j]->name) == 0)
				{
					desiredFolder = desiredFolder->sons[j];
					isDesiredFolderFinded = 1;
					break;
				}
			}
			if (!isDesiredFolderFinded) return nullptr;
		}
	}
	return desiredFolder;
}

void DeleteFolder(Folder* folder)
{
	if (currentFolder == folder) currentFolder = rootFolder;
	if (folder != rootFolder)
	{
		folder->parent->sonsCount -= 1;
	}
	for (int i = 0; i < folder->sonsCount; i++)
	{
		DeleteFolder(folder->sons[i]);
	}
	for (int i = 0; i < folder->filesCount; i++)
	{
		availableDiskSize += folder->files[i]->size;
		free(folder->files[i]);
	}
	free(folder);
	return;
}

int GetFileCountRec(Folder* folder)
{
	int count = 0;
	for (int i = 0; i < folder->sonsCount; i++)
	{
		count += GetFileCountRec(folder->sons[i]);
	}
	return count + folder->filesCount;
}
#pragma endregion

int diskSize;
bool isCreated;

int my_create(int disk_size)
{
	if (isCreated) return 0;
	diskSize = disk_size;
	availableDiskSize = diskSize;
	rootFolder = CreateFolder("/", nullptr);
	currentFolder = rootFolder;
	isCreated = 1;
	//PrintFolderStructure(rootFolder);
	return 1;
}

int my_destroy()
{
	if (!isCreated) return 0;
	DeleteFolder(rootFolder);
	rootFolder = nullptr;
	//PrintFolderStructure(rootFolder);
	return 1;
}

int my_create_dir(const char* path)
{
	if (!isCreated) return 0;
	ParsedPath* parsedPath = GetParsedPath(path);
	//PrintParsedPath(parsedPath);
	parsedPath->tokenCount -= 1;
	Folder* parentFolder = FindFolder(parsedPath);
	if (parentFolder == nullptr) return 0;
	//cout << parsedPath->path[parsedPath->tokenCount] << endl;
	parentFolder->sons[parentFolder->sonsCount] = CreateFolder(parsedPath->path[parsedPath->tokenCount], parentFolder);
	if (parentFolder->sons[parentFolder->sonsCount] == nullptr) return 0;
	parentFolder->sonsCount++;
	//PrintFolderStructure(rootFolder);
	return 1;
}

int my_create_file(const char* path, int file_size)
{
	if (!isCreated) return 0;
	if (availableDiskSize - file_size < 0) return 0;
	availableDiskSize -= file_size;
	ParsedPath* parsedPath = GetParsedPath(path);
	//PrintParsedPath(parsedPath);
	parsedPath->tokenCount -= 1;
	Folder* parentFolder = FindFolder(parsedPath);
	if (parentFolder == nullptr) return 0;
	//cout << parsedPath->path[parsedPath->tokenCount] << endl;
	parentFolder->files[parentFolder->filesCount] = CreateFile(parsedPath->path[parsedPath->tokenCount], file_size, parentFolder);
	if (parentFolder->files[parentFolder->filesCount] == nullptr) return 0;
	parentFolder->filesCount++;
	//PrintFolderStructure(rootFolder);
	return 1;
}

int my_remove(const char* path, int recursive)
{
	if (!isCreated) return 0;
	ParsedPath* parsedPath = GetParsedPath(path);
	Folder* folder = FindFolder(parsedPath);
	if (folder == nullptr) return 0;
	if (recursive)
	{
		DeleteFolder(folder);
	}
	else
	{
		if (folder->filesCount || folder->sonsCount) return 0;
		DeleteFolder(folder);
	}
	PrintFolderStructure(rootFolder);
	return 1;
}

int my_change_dir(const char* path)
{
	if (!isCreated) return 0;
	ParsedPath* parsedPath = GetParsedPath(path);
	Folder* folder = FindFolder(parsedPath);
	currentFolder = folder;
	//cout << currentFolder->name << endl;
	if (folder == nullptr) return 0;
	return 1;
}

void my_get_cur_dir(char* dst)
{
	Folder* desiredFolder = currentFolder;
	ParsedPath* parsedPath = (ParsedPath*)malloc(sizeof(ParsedPath));
	parsedPath->path = (char**)malloc(countMemoryItemsSize * sizeof(char*));
	if (parsedPath->path)
	{
		for (int i = 0; i < countMemoryItemsSize; i++)
			parsedPath->path[i] = (char*)malloc(countMemoryItemsSize * sizeof(char));
	}
	parsedPath->tokenCount = 0;
	strcpy(parsedPath->path[parsedPath->tokenCount], desiredFolder->name);
	while (desiredFolder != rootFolder)
	{
		desiredFolder = desiredFolder->parent;
		strcpy(parsedPath->path[++parsedPath->tokenCount], desiredFolder->name);
	}
	int symbolCount = 0;
	for (int i = parsedPath->tokenCount - 1; i >= 0; i--)
	{
		dst[symbolCount] = '/';
		symbolCount++;
		for (int j = 0; j < Length(parsedPath->path[i]); j++)
		{
			dst[symbolCount] = parsedPath->path[i][j];
			symbolCount++;
		}
	}
	dst[symbolCount] = '\0';
	return;
}

int my_files_count(const char* path)
{
	if (!isCreated) return -1;
	ParsedPath* parsedPath = GetParsedPath(path);
	Folder* folder = FindFolder(parsedPath);
	if (folder == nullptr) return -1;
	return GetFileCountRec(folder);
}

void setup_file_manager(file_manager_t* fm)
{
	fm->create = my_create;
	fm->destroy = my_destroy;
	fm->create_dir = my_create_dir;
	fm->create_file = my_create_file;
	fm->remove = my_remove;
	fm->change_dir = my_change_dir;
	fm->get_cur_dir = my_get_cur_dir;
	fm->files_count = my_files_count;
}