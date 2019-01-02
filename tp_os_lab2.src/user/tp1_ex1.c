#include <oslib.h>
#include <uart.h>

void code1()
{
    int   k;
    uint8 id=task_id();

    while(1) {
		uart_printf(_UART0, "tache %d\n",id);
        for (k=0;k<500000;k++);
    }
}

void code2()
{
    int   k;
    uint8 id=task_id();

    while(1) {
		uart_printf(_UART0, "tache %d\n",id);

        for (k=0;k<500000;k++);
    }
}

void code3()
{
    int   k;
    uint8 id=task_id();

    while(1) {
		uart_printf(_UART0, "tache %d\n",id);

        for (k=0;k<500000;k++);
    }
}

void code4()
{
    int   k;
    uint8 id=task_id();

    while(1) {
		uart_printf(_UART0, "tache %d\n",id);

        for (k=0;k<500000;k++);
    }
}



int main()
{
    task_new(code1, 256);   // tache 1
    task_new(code2, 256);   // tache 2
    task_new(code3, 256);   // tache 3
    task_new(code4, 256);   // tache 4

    os_start();

    return 0;
}
