#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void write_data()
{
    FILE    *fp;

    fp = fopen("inst.txt", "w");

    fprintf(fp, "%s %d %x %c\n","JSUB", 3, 0x48, '1');
    fprintf(fp, "%s %d %x %c\n", "LDA", 3, 0x00, '1');
    fprintf(fp,"%s %d %x %c\n", "COMP", 3, 0x28, '1');
    fprintf(fp,"%s %d %x %c\n", "JEQ", 3, 0x30, '1');
    fprintf(fp,"%s %d %x %c\n", "J", 3, 0x3C, '1');
    fprintf(fp,"%s %d %x %c\n", "STA", 3, 0x0C, '1');
    fprintf(fp,"%s %d %x %c\n", "CLEAR", 2, 0xB4, '1');
    fprintf(fp,"%s %d %x %c\n", "LDT", 3, 0x74, '1');
    fprintf(fp,"%s %d %x %c\n", "TD", 3, 0xE0, '1');
    fprintf(fp,"%s %d %x %c\n", "RD", 3, 0xD8, '1');
    fprintf(fp,"%s %d %x %c\n", "COMPR", 2, 0xA0, '2');
    fprintf(fp,"%s %d %x %c\n", "TIXR", 2, 0xB8, '1');
    fprintf(fp,"%s %d %x %c\n", "JLT", 3, 0x38, '1');
    fprintf(fp,"%s %d %x %c\n", "RSUB", 3, 0x4C, '0');
    fprintf(fp,"%s %d %x %c\n", "WD", 3, 0xDC, '1');
    fprintf(fp,"%s %d %x %c\n", "STL", 3, 0x14, '1');
    fprintf(fp,"%s %d %x %c\n", "STX", 3, 0x10, '1');
    fprintf(fp,"%s %d %x %c\n", "LDCH", 3, 0x50, '1');
    fprintf(fp,"%s %d %x %c\n", "STCH", 3, 0x54, '1');


    fclose(fp);

    return;
}


int main ()
{
    write_data();
}
