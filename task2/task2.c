#include <stdio.h>
#define N 5

int printMatrix()
{
    printf("Matrix %dx%d:\n", N, N);
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            int num = i * N + j + 1;
            printf("%d\t", num);
        }
        printf("\n");
    }
}

void reverseArray()
{
    int array[N];

    printf("Please enter an array with size %d: ", N);
    for (int i = 0; i < N; i++)
    {
        scanf("%d", &array[i]);
    }

    for (int i = 0; i < N / 2; i++)
    {
        array[i] ^= array[N - i - 1];
        array[N - i - 1] ^= array[i];
        array[i] ^= array[N - i - 1];
    }

    for (int i = 0; i < N; i++)
    {
        printf("%d ", array[i]);
    }
    printf("\n");
}

void printTriangle()
{
    printf("Triangle of %d size:\n", N);
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            char num = 0;
            if (i + j >= N - 1)
            {
                num = 1;
            }
            printf("%d ", num);
        }
        printf("\n");
    }
}

void printSpiral()
{
    int array[N][N] = {0};
    int num = 2;
    int i = 0, j = 0;
    int direction = 0; // 0 - right, 1 - down, 2 - left, 3 - up
    int top = N - 1, bottom = 1;
    int right = N - 1, left = 0;

    array[0][0] = 1;
    while (num <= N * N)
    {
        switch (direction)
        {
        case 0:
            j++;
            if (j > right)
            {
                j--;
                right = j - 1;
                direction = 1;
                continue;
            }
            break;
        case 1:
            i++;
            if (i > top)
            {
                i--;
                top = i - 1;
                direction = 2;
                continue;
            }
            break;
        case 2:
            j--;
            if (j < left)
            {
                j++;
                left = j + 1;
                direction = 3;
                continue;
            }
            break;
        case 3:
            i--;
            if (i < bottom)
            {
                i++;
                bottom = i + 1;
                direction = 0;
                continue;
            }
            break;
        default:
            break;
        }

        array[i][j] = num;
        num++;

    };

    printf("Spiral %dx%d:\n", N, N);
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            printf("%d\t", array[i][j]);
        }
        printf("\n");
    }
}

int main()
{
    printMatrix(N);
    reverseArray(N);
    printTriangle();
    printSpiral();

    getchar();

    return 0;
}