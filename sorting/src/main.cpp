// SOUND: https://stackoverflow.com/questions/72296718/is-there-a-way-in-c-to-start-playing-a-sound-of-a-given-frequency


// **PREPROCESSOR**
#define SORT_SIZE 1024
#define PITCH 4
#define VOLUME 2000



#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include <windows.h>
#include <dwmapi.h>
#include "../resource.h"
#include "array.h"
#include <thread>
#include <iostream>
#include <random>
#include <memory>
#include <chrono>
#include "audio.h"


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

HWND hwnd;

int width;
int height;
int* sort_template;
uint32_t* mem;
int* sort;
HDC hdc;
BITMAPINFO bmi;

bool isSound = true;

int set_cursor = 0;


void swap(int* a, int* b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}


unsigned long createRGB(int r, int g, int b)
{
	return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
}

void visualize(int x, bool whole, bool red, bool green = false) {

	uint32_t mxH = sort[x];

	if (!red && !whole) {
		std::this_thread::sleep_for(std::chrono::nanoseconds(30));
		if(isSound)
			sound(mxH * PITCH);
	}

	for (int y = 0; y < height; y++) {
		uint32_t* m = mem;
		m += (y * width) + x;

		if (y <= mxH) {
			int hue = (mxH * 255) / 512;
			int fade = (y * 255) / SORT_SIZE;

			*m = green ? 0x00FF00 : (red ? 0xFF0000 : 0xFFFFFF);//createRGB(hue, 255-hue, fade);
		}
		else {
			*m = 0x00;
		}
	}

}

void updateBus() {

	

	RECT r{};
	GetClientRect(hwnd, &r);
	int w = r.right - r.left;
	int h = r.bottom - r.top;
	StretchDIBits(hdc, 0,0, w, h, 0,0, width, height, mem, &bmi, DIB_RGB_COLORS, SRCCOPY);
	//SetDIBitsToDevice(hdc, 0, 0, width, height, 0, 0, 0, height, mem, &bmi, DIB_RGB_COLORS);
}

void partVisualize(int x, bool red, bool green = false) {

	visualize(x, false, red, green);


	updateBus();
}

void fullVisualize() {
	for (int x = 0; x < SORT_SIZE; x++) {
		visualize(x, true, false);
	}

	updateBus();
}


void merge(int array[], int const left, int const mid, int const right) {


	auto const subArrayOne = mid - left + 1;
	auto const subArrayTwo = right - mid;

	// Create temp arrays
	auto* leftArray = new int[subArrayOne],
		* rightArray = new int[subArrayTwo];

	// Copy data to temp arrays leftArray[] and rightArray[]
	for (auto i = 0; i < subArrayOne; i++)
		leftArray[i] = array[left + i];
	for (auto j = 0; j < subArrayTwo; j++)
		rightArray[j] = array[mid + 1 + j];

	auto indexOfSubArrayOne
		= 0, // Initial index of first sub-array
		indexOfSubArrayTwo
		= 0; // Initial index of second sub-array
	int indexOfMergedArray
		= left; // Initial index of merged array

	// Merge the temp arrays back into array[left..right]
	while (indexOfSubArrayOne < subArrayOne
		&& indexOfSubArrayTwo < subArrayTwo) {
		if (leftArray[indexOfSubArrayOne]
			<= rightArray[indexOfSubArrayTwo]) {


			array[indexOfMergedArray] = leftArray[indexOfSubArrayOne];
			set_cursor = indexOfMergedArray;
			partVisualize(indexOfMergedArray, false);

			indexOfSubArrayOne++;
		}
		else {
			array[indexOfMergedArray] = rightArray[indexOfSubArrayTwo];
			set_cursor = indexOfMergedArray;
			partVisualize(indexOfMergedArray, false);
			indexOfSubArrayTwo++;
		}
		partVisualize(indexOfMergedArray+1, true);
		partVisualize(indexOfMergedArray,false);
		indexOfMergedArray++;
	}
	// Copy the remaining elements of
	// left[], if there are any
	while (indexOfSubArrayOne < subArrayOne) {
		array[indexOfMergedArray] = leftArray[indexOfSubArrayOne];
		set_cursor = indexOfMergedArray;
		partVisualize(indexOfMergedArray, false);

		indexOfSubArrayOne++;

		partVisualize(indexOfMergedArray + 1, true);
		partVisualize(indexOfMergedArray, false);
		indexOfMergedArray++;
	}
	// Copy the remaining elements of
	// right[], if there are any
	while (indexOfSubArrayTwo < subArrayTwo) {


		array[indexOfMergedArray] = rightArray[indexOfSubArrayTwo];
		set_cursor = indexOfMergedArray;
		partVisualize(indexOfMergedArray, false);
		indexOfSubArrayTwo++;

		partVisualize(indexOfMergedArray + 1, true);
		partVisualize(indexOfMergedArray, false);
		indexOfMergedArray++;
	}

	partVisualize(indexOfMergedArray, false);
	delete[] leftArray;
	delete[] rightArray;
}


int partition(int arr[], int start, int end)
{

	int pivot = arr[start];

	int count = 0;
	for (int i = start + 1; i <= end; i++) {
		if (arr[i] <= pivot)
			count++;
	}

	// Giving pivot element its correct position

	int pivotIndex = start + count;

	std::swap(arr[pivotIndex], arr[start]);
	fullVisualize();

	// Sorting left and right parts of the pivot element
	int i = start, j = end;

	while (i < pivotIndex && j > pivotIndex) {

		while (arr[i] <= pivot) {
			i++;
		}

		while (arr[j] > pivot) {
			j--;
		}

		if (i < pivotIndex && j > pivotIndex) {
			std::swap(arr[i++], arr[j--]);
			fullVisualize();
		}
	}

	return pivotIndex;
}

void quickSort(int arr[], int start, int end)
{

	// base case
	if (start >= end)
		return;

	// partitioning the array
	int p = partition(arr, start, end);

	// Sorting the left part
	quickSort(arr, start, p - 1);

	// Sorting the right part
	quickSort(arr, p + 1, end);

}

void mergeSort(int array[], int const begin, int const end)
{
	if (begin >= end)
		return; // Returns recursively

	auto mid = begin + (end - begin) / 2;

	mergeSort(array, begin, mid);

	mergeSort(array, mid + 1, end);

	merge(array, begin, mid, end);

}

void goThread() {

	std::this_thread::sleep_for(std::chrono::seconds(1));

	while (1) {
		
		for (int i = 0; i < SORT_SIZE; i++) {
			sort[i] = (i / 2);
		}


		srand(time(NULL));

		// Start from the last element and swap
		// one by one. We don't need to run for
		// the first element that's why i > 0

		int n = SORT_SIZE;

		for (int i = n - 1; i > 0; i--)
		{
			// Pick a random index from 0 to i
			int j = rand() % (i + 1);

			// Swap arr[i] with the element
			// at random index
			swap(&sort[i], &sort[j]);


			partVisualize(i, 0, 0);
		}
		/*
		
		for (int i = 0; i < SORT_SIZE; i++) {

			sort[i] = sort_template[i];
		}

		*/

		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		sound(0);

		std::this_thread::sleep_for(std::chrono::seconds(1));

		fullVisualize();

		mergeSort(sort, 0, SORT_SIZE-1);


		sound(0);


		fullVisualize();

		for (int i = 0; i < SORT_SIZE; i++) {
			partVisualize(i, 0, 1);
		}

		fullVisualize();
		sound(0);

		std::this_thread::sleep_for(std::chrono::seconds(1));

	}


}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {

	const char* CLASS_NAME = "MainWindowClass";

	WNDCLASSEX wc = { 0 };
	wc.hbrBackground = CreateSolidBrush(RGB(0,0,0));
	//wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = CLASS_NAME;
	wc.hCursor = LoadCursor(0, IDC_ARROW);

	RegisterClassEx(&wc);

	width = SORT_SIZE;
	height = 512;

	RECT ws = { 0, 0, 1280, 720 };
	AdjustWindowRectEx(&ws, WS_OVERLAPPEDWINDOW, 0, 0);

	int w_width = ws.right - ws.left;
	int w_height = ws.bottom - ws.top;

	hwnd = CreateWindowEx(WS_EX_DLGMODALFRAME, CLASS_NAME, "", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, w_width, w_height, 0, 0, 0, 0);


	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	hdc = GetDC(hwnd);

	int arrSize = width * height * 4;


	mem = (uint32_t*)calloc(arrSize, sizeof(uint32_t));



	if (mem == NULL) {
		return 0;
	}

	//sort_template = new int[SORT_SIZE];


	sort = new int[SORT_SIZE];
	
	if (audio_init()) { return 1; }


	std::thread t{ goThread };

	MSG msg = { 0 };
	bool running = true;

	while (running) {

		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT) { running = false; }
			
		}
	}
	t.detach();
	return 0;
}

struct ACCENTPOLICY
{
	int na;
	int nf;
	int nc;
	int nA;
};
struct WINCOMPATTRDATA
{
	int na;
	PVOID pd;
	ULONG ul;
};


bool fullscreen = false;
bool transparent = false;
RECT prevCoordinates;

void SetBlur(HWND hwnd) {
	const HINSTANCE hm = LoadLibrary("user32.dll");
	if (hm)
	{
		typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WINCOMPATTRDATA*);

		const pSetWindowCompositionAttribute SetWindowCompositionAttribute = (pSetWindowCompositionAttribute)GetProcAddress(hm, "SetWindowCompositionAttribute");
		if (SetWindowCompositionAttribute)
		{
			ACCENTPOLICY policy = { 3,0,155,0 }; // and even works 4,0,155,0 (Acrylic blur)
			WINCOMPATTRDATA data = { 19, &policy,sizeof(ACCENTPOLICY) };
			SetWindowCompositionAttribute(hwnd, &data);
		}
		FreeLibrary(hm);
	}

}

void ResetBlur(HWND hwnd) {
	const HINSTANCE hm = LoadLibrary("user32.dll");
	if (hm)
	{
		typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WINCOMPATTRDATA*);

		const pSetWindowCompositionAttribute SetWindowCompositionAttribute = (pSetWindowCompositionAttribute)GetProcAddress(hm, "SetWindowCompositionAttribute");
		if (SetWindowCompositionAttribute)
		{
			ACCENTPOLICY policy = { 0,0,155,0 }; // and even works 4,0,155,0 (Acrylic blur)
			WINCOMPATTRDATA data = { 19, &policy,sizeof(ACCENTPOLICY) };
			SetWindowCompositionAttribute(hwnd, &data);
		}
		FreeLibrary(hm);
	}

}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
		case WM_CREATE: {

			BOOL enable = TRUE;


			DwmSetWindowAttribute(hwnd, 20, &enable, sizeof(enable));

			break;
		}
		case WM_KEYDOWN: {
			if (wparam == 'M') {
				isSound = !isSound;
				sound(0);
			}
			if (wparam == 'T') {
				transparent = !transparent;

				if (transparent) {
					//SetWindowLong(hwnd, GWL_STYLE, WS_CAPTION | WS_SYSMENU | WS_VISIBLE);

					SetBlur(hwnd);

					MARGINS m = { 1000, 1000, 1000, 1000 };
					DwmExtendFrameIntoClientArea(hwnd, &m);

				}
				else {
					//SetWindowLong(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
					ResetBlur(hwnd);


					MARGINS m = { 0,0,0,0 };
					DwmExtendFrameIntoClientArea(hwnd, &m);
				}
			}

			if (wparam == VK_F11) {
				fullscreen = !fullscreen;

				if (fullscreen) {
					GetWindowRect(hwnd, &prevCoordinates);

					int fullX = GetSystemMetrics(SM_CXSCREEN)-1;
					int fullY = GetSystemMetrics(SM_CYSCREEN)-1;

					SetWindowPos(hwnd, 0, 0, 0, fullX, fullY, 0);

					SetWindowLong(hwnd, GWL_EXSTYLE, 0);

					SetWindowLong(hwnd, GWL_STYLE, WS_VISIBLE);

					SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

					MARGINS m = { 0,0,0,0 };
					DwmExtendFrameIntoClientArea(hwnd, &m);

				}
				else {

					int posX = prevCoordinates.left;
					int posY = prevCoordinates.top;

					int sizeX = prevCoordinates.right - posX;
					int sizeY = prevCoordinates.bottom - posY;

					SetWindowPos(hwnd, 0, posX, posY, sizeX, sizeY, 0);

					SetWindowLong(hwnd, GWL_STYLE, WS_VISIBLE);

					SetWindowLong(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);

					SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_DLGMODALFRAME);

					if (transparent) {

						MARGINS m = { 1000, 1000, 1000, 1000 };
						DwmExtendFrameIntoClientArea(hwnd, &m);
					}
				}
			}
			break;
		}
		case WM_DESTROY: {
			PostQuitMessage(0);
			return 0;
		}
		default: {
			return DefWindowProc(hwnd, msg, wparam, lparam);
		}
	}
}
