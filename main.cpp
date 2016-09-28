#include <omp.h>
#include <fstream>
#include <ctime>
#include <iostream>
using namespace std;
// ����� ��������� ���������� � ������
// #pragma omp parallel - ��������� ������� ������������� ���������� ����
// shared(������ ����������) � ������ ����� ���������� ��� ���� ������� ������ ������
// private(������ ����������) �	������ ����������, ������� ����� ���������� � ������ ������ ������
// reduction(�������� : ������ ����������) � �������� �������� � ������ ����� ���������� 
// ��������� ���������� ������� � ����� ���������� �� ����� ����� 
// ��������� ������������ �� ����� ���������� ���� �������� � ���� ���������� � ������ ���������: #pragma omp atomic


//���������� ������ �� �������
void downHeap(int *a, long k, long n) {
	int new_elem = a[k];
	long child;
	// ���� � a[k] ���� ���� - �������� �������� �� ���
	while (k <= n / 2) {  		
		child = 2 * k;
		//  �������� �������� ���� 
		if (child < n && a[child] < a[child + 1])
			child++;
		if (new_elem >= a[child]) break;
		// ����� 
		a[k] = a[child]; 	// ��������� ���� ������ 
		k = child;
	}
	a[k] = new_elem;
}

//���������� ���������
void sort(int *a, int size)
{
	// �������� �������
	for (int i = size / 2; i >= 0; i--) {
		downHeap(a, i, size - 1);
	}
	// 
	for (int i = size - 1; i > 0; i--)
	{
		int temp = a[i];
		a[i] = a[0];
		a[0] = temp;

		downHeap(a, 0, i - 1);
	}
}

int isMin(int x, int y)
{
	if (x <= y) return x;
	else return y;
}

//������� ���� ������������� ����� ���� � ������ ������ �������
void merge(int *A, int *B, int l, int m, int r)
//A - ����������� ������, B - ����� ��� �������, l - ������ ������� ������ �����, r - ��������� ������� ������ �����, m - ��������� ������� ������ �����
{
	int i = l;
	int j = m + 1;
	int k = l;
	//��������� ����������� �������� � B ���� �� �������� ���� �� �������������������
	while ((i <= m) && (j <= r))
	{
		if (A[i]<A[j])
		{
			B[k] = A[i];
			i++;
		}
		else {
			B[k] = A[j];
			j++;
		}
		k++;
	}
	//����������� �������, ���� ������� �������
	while (i <= m)
	{
		B[k] = A[i];
		k++;
		i++;
	}
	while (j <= r)
	{
		B[k] = A[j];
		k++;
		j++;
	}
	//��������������� ����� ������� � B
}

//���������� ��������, ����������������
void mp_sort(int* A, int* B, int N, int P)
//A - ����������� ������, B - ����� ��� �������, N - ������ �������, P - ����� ����������� ����������� �������
{
	int *tA = A;
	int *tB = B;
	int *t = NULL;
	// 1) ��������� ������� �� ����� � ���������� �� �� �����������
	int i;
	int part_size = (int)ceil((float)N / (float)P);
	int count = 0;
#pragma omp parallel shared(P, tA, tB, N, part_size)
#pragma omp for private(i) reduction(+:count) 
	for (i = 0; i < P; i++)
	{
		int balance = N - i * part_size;
		int arraySize = isMin(part_size, balance);
		sort(tA + i * part_size, arraySize);
		count += isMin(part_size, balance);
	}
	int r2, m;
	int count2 = 0;
	while (part_size < (2 * N))
	{
#pragma omp parallel shared(P, tA, tB, N, part_size)
#pragma omp for private(i,r2,m)
		for (i = 0; i<N; i += part_size)
		{
			r2 = isMin(i + part_size - 1, N - 1);
			m = ((i + i + part_size - 1) >> 1);
			merge(tA, tB, i, m, r2);
#pragma omp atomic
			count2 += part_size;
		}
#pragma omp crit#ical
		part_size *= 2;
		t = tA;
		tA = tB;
		tB = t;
		t = NULL;
	}
	if (tA != A)
	{
		//���������� �������� � �������� ������ �
		omp_lock_t lock;
		omp_init_lock(&lock);
		int k;
#pragma omp parallel shared(A,B,N)
#pragma omp for private(k)
		for (k = 0; k< N; k++)
		{
			A[k] = B[k];
		}
	}
}


bool test(int* A, int* B, int size)
{
	for (int i = 0; i < size; i++)
		if (A[i] != B[i])
		{
			return false;
		}
	return true;
}


int main()
{
	setlocale(LC_ALL, ".ACP");

	cout << "������� ������ �������: ";
	int n;
	cin >> n;

	int *mas = new int[n];
	int *mas1 = new int[n];
	int *mas2 = new int[n];

	for (int i = 0; i < n; i++)
	{
		int r = rand() % 100;
		mas[i] = r;
		mas1[i] = r;
		// cout << mas[i] << endl;
	}
	cout << endl;

	long t = clock();
	sort(mas, n);
	cout << "���������������� ��������: " << clock() - t << " ��" << endl;

	t = clock();
	mp_sort(mas1, mas2, n, 16);
	cout << "������������ ��������: " << clock() - t << " ��" << endl;

	if (test(mas, mas1, n))
		cout << "true" << endl;
	else
		cout << "false" << endl;

	delete[] mas;
	delete[] mas1;
	delete[] mas2;

	system("pause");
}