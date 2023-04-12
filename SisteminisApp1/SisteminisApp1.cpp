#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <Windows.h>
using namespace std;

void functionMenu(HANDLE& file, DWORD& size, LPVOID& data, DWORD& bytes, string& album, string& artist, string& title);
void returnMp3Metadata(HANDLE& file, DWORD& size, LPVOID& data, DWORD& bytes, string& album, string& artist, string& title);
void openFile(HANDLE& file);
void getFileSize(HANDLE& file, DWORD& size);
void allocateMemory(HANDLE& file, DWORD& size, LPVOID& data);
void readIntoMemory(HANDLE& file, DWORD& size, LPVOID& data, DWORD& bytes);
unique_ptr<char[]> readFileData(HANDLE& file, DWORD& size, LPVOID& data);
bool checkTagExists(const unique_ptr<char[]>& fileD, DWORD& size);
string getAlbum(const unique_ptr<char[]>& fileD, DWORD size);
string getArtist(const unique_ptr<char[]>& fileD, DWORD size);
string getTitle(const unique_ptr<char[]>& fileD, DWORD size);
void displayMetadata(string& album, string& artist, string& title);
void changeMp3Metadata(HANDLE& file, DWORD& size, LPVOID& data, DWORD& bytes, string& album, string& artist, string& title);

int main()
{
	HANDLE mp3File;
	DWORD fileSize;
	LPVOID metadata;
	DWORD readBytes;
	string album;
	string artist;
	string title;

	functionMenu(mp3File, fileSize, metadata, readBytes, album, artist, title);

	VirtualFree(metadata, 0, MEM_RELEASE);

	CloseHandle(mp3File);

	return 0;
}

void functionMenu(HANDLE& file, DWORD& size, LPVOID& data, DWORD& bytes, string& album, string& artist, string& title)
{
	int input;
	printf("Enter 1 to display metadata, 2 to change metadata: \n");
	scanf_s("%i", &input);
	switch (input)
	{
	case 1:
		returnMp3Metadata(file, size, data, bytes, album, artist, title);
		break;
	case 2:
		changeMp3Metadata(file, size, data, bytes, album, artist, title);
		break;
	default:
		break;
	}

}

void returnMp3Metadata(HANDLE& file, DWORD& size, LPVOID& data, DWORD& bytes, string& album, string& artist, string& title)
{
	openFile(file);
	getFileSize(file, size);
	allocateMemory(file, size, data);
	readIntoMemory(file, size, data, bytes);

	unique_ptr<char[]> fileData = readFileData(file, size, data);

	if (!checkTagExists(fileData, size)) {
		printf("File does not contain TAG metadata\n");
		return;
	}

	album = getAlbum(fileData, size);
	artist = getArtist(fileData, size);
	title = getTitle(fileData, size);

	displayMetadata(album, artist, title);
}

void openFile(HANDLE& file)
{
	file = CreateFile(L"bensound-far.mp3", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE)
	{
		printf("Failed to open file\n");
	}
}

void getFileSize(HANDLE& file, DWORD& size)
{
	size = GetFileSize(file, NULL);
	if (size == INVALID_FILE_SIZE) {
		printf("Failed to get file size\n");
		CloseHandle(file);
	}
}

void allocateMemory(HANDLE& file, DWORD& size, LPVOID& data)
{
	data = VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN, PAGE_READWRITE);
	if (data == NULL) {
		printf("Failed to allocate memory");
		CloseHandle(file);
	}
}

void readIntoMemory(HANDLE& file, DWORD& size, LPVOID& data, DWORD& bytes)
{
	if (!ReadFile(file, data, size, &bytes, NULL)) {
		printf("Failed to read file\n");
		VirtualFree(data, 0, MEM_RELEASE);
		CloseHandle(file);
	}
}

unique_ptr<char[]> readFileData(HANDLE& file, DWORD& size, LPVOID& data) {
	unique_ptr<char[]> fileData(new char[size + 1]);
	memcpy(fileData.get(), data, size);
	return fileData;
}

bool checkTagExists(const unique_ptr<char[]>& fileD, DWORD& size) {
	return string(fileD.get() + size - 128, 3) == "TAG";
}


string getAlbum(const unique_ptr<char[]>& fileD, DWORD size) {
	return string(fileD.get() + size - 65, fileD.get() + size - 35);
}

string getArtist(const unique_ptr<char[]>& fileD, DWORD size) {
	return string(fileD.get() + size - 95, fileD.get() + size - 65);
}

string getTitle(const unique_ptr<char[]>& fileD, DWORD size) {
	return string(fileD.get() + size - 125, fileD.get() + size - 95);
}

void displayMetadata(string& album, string& artist, string& title) {
	cout << "Album: " << album << endl;
	cout << "Artist: " << artist << endl;
	cout << "Title: " << title << endl;
}

void changeMp3Metadata(HANDLE& file, DWORD& size, LPVOID& data, DWORD& bytes, string& album, string& artist, string& title)
{
	cout << "Enter new album name: ";
	cin.ignore();
	getline(cin, album);
	cout << "Enter new artist name: ";
	getline(cin, artist);
	cout << "Enter new title: ";
	getline(cin, title);

	openFile(file);
	getFileSize(file, size);
	allocateMemory(file, size, data);
	readIntoMemory(file, size, data, bytes);

	CloseHandle(file);

	album.resize(30, ' ');
	artist.resize(30, ' ');
	title.resize(30, ' ');

	memcpy((char*)data + size - 65, album.c_str(), 30);
	memcpy((char*)data + size - 95, artist.c_str(), 30);
	memcpy((char*)data + size - 125, title.c_str(), 30);

	file = CreateFile(L"bensound-far.mp3", GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE)
	{
		printf("Failed to open file for writing metadata\n");
		return;
	}

	if (!WriteFile(file, data, size, &bytes, NULL))
	{
		printf("Failed to write the updated metadata to the file\n");
	}
	else
	{
		printf("Successfully updated the metadata\n");
	}
}