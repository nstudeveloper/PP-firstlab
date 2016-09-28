#include <omp.h>
#include <fstream>
#include <ctime>
#include <iostream>
using namespace std;
// Ћюба€ директива начинаетс€ с прагмы
// #pragma omp parallel - директива задани€ параллельного выполнени€ кода
// shared(список переменных) Ц список общих переменных дл€ всех потоков данной секции
// private(список переменных) Ц	список переменных, которые будут локальными в каждом потоке секции
// reduction(оператор : список переменных) Ц оператор сведени€ и список общих переменных 
// ƒиректива блокировки доступа к общей переменной из левой части 
// оператора присваивани€ на врем€ выполнени€ всех действий с этой переменной в данном операторе: #pragma omp atomic


//построение дерева из массива
void downHeap(int *a, long k, long n) {
	int new_elem = a[k];
	long child;
	// ѕока у a[k] есть дети - выбираем большего из них
	while (k <= n / 2) {  		
		child = 2 * k;
		//  выбираем большего сына 
		if (child < n && a[child] < a[child + 1])
			child++;
		if (new_elem >= a[child]) break;
		// иначе 
		a[k] = a[child]; 	// переносим сына наверх 
		k = child;
	}
	a[k] = new_elem;
}

//сортировка пирамидой
void sort(int *a, int size)
{
	// ѕоловину массива
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

//—ли€ние двух расположенных р€дом друг с другом частей массива
void merge(int *A, int *B, int l, int m, int r)
//A - сортируемый массив, B - буфер дл€ сли€ни€, l - первый элемент первой части, r - последний элемент второй части, m - последний элемент первой части
{
	int i = l;
	int j = m + 1;
	int k = l;
	//¬ставл€ть минимальные элементы в B пока не кончитс€ одна из последовательностей
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
	//—копировать остаток, если таковой имеетс€
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
	//ќтсортированна€ часть остаЄтс€ в B
}

//—ортировка сли€нием, распараллеленна€
void mp_sort(int* A, int* B, int N, int P)
//A - сортируемый массив, B - буфер дл€ сли€ни€, N - размер массива, P - число параллельно выполн€емых потоков
{
	int *tA = A;
	int *tB = B;
	int *t = NULL;
	// 1) –азбиение массива на блоки и сортировка их по отдельности
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
		//ѕереписать элементы в исходный массив ј
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

	cout << "¬ведите размер массива: ";
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
	cout << "ѕоследовательный алгоритм: " << clock() - t << " мс" << endl;

	t = clock();
	mp_sort(mas1, mas2, n, 16);
	cout << "ѕараллельный алгоритм: " << clock() - t << " мс" << endl;

	if (test(mas, mas1, n))
		cout << "true" << endl;
	else
		cout << "false" << endl;

	delete[] mas;
	delete[] mas1;
	delete[] mas2;

	system("pause");
}