/*
 * 화일명 : my_assembler_20182630.c
 * 설  명 : 이 프로그램은 SIC/XE 머신을 위한 간단한 Assembler 프로그램의 메인루틴으로,
 * 입력된 파일의 코드 중, 명령어에 해당하는 OPCODE를 찾아 출력한다.
 * 파일 내에서 사용되는 문자열 "00000000"에는 자신의 학번을 기입한다.
 */

/*
 *
 * 프로그램의 헤더를 정의한다.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

// 파일명의 "00000000"은 자신의 학번으로 변경할 것.
#include "my_assembler_20182630.h"

/* ----------------------------------------------------------------------------------
 * 설명 : 사용자로 부터 어셈블리 파일을 받아서 명령어의 OPCODE를 찾아 출력한다.
 * 매계 : 실행 파일, 어셈블리 파일
 * 반환 : 성공 = 0, 실패 = < 0
 * 주의 : 현재 어셈블리 프로그램의 리스트 파일을 생성하는 루틴은 만들지 않았다.
 *           또한 중간파일을 생성하지 않는다.
 * ----------------------------------------------------------------------------------
 */
int main(int args, char *arg[])
{
    if (init_my_assembler() < 0)
    {
        printf("init_my_assembler: 프로그램 초기화에 실패 했습니다.\n");
        return -1;
    }

    if (assem_pass1() < 0)
    {
        printf("assem_pass1: 패스1 과정에서 실패하였습니다.  \n");
        return -1;
    }
    // make_opcode_output("output_20182630");

    make_symtab_output("symtab_20182630");
    make_literaltab_output("literaltab_20182630");
    if (assem_pass2() < 0)
    {
        printf(" assem_pass2: 패스2 과정에서 실패하였습니다.  \n");
        return -1;
    }

    make_objectcode_output("output_20182630");

    return 0;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 프로그램 초기화를 위한 자료구조 생성 및 파일을 읽는 함수이다.
 * 매계 : 없음
 * 반환 : 정상종료 = 0 , 에러 발생 = -1
 * 주의 : 각각의 명령어 테이블을 내부에 선언하지 않고 관리를 용이하게 하기
 *           위해서 파일 단위로 관리하여 프로그램 초기화를 통해 정보를 읽어 올 수 있도록
 *           구현하였다.
 * ----------------------------------------------------------------------------------
 */
int init_my_assembler(void)
{
    int result;

    if ((result = init_inst_file("inst.txt")) < 0)
        return -1;
    if ((result = init_input_file("input.txt")) < 0)
        return -1;
    return result;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 머신을 위한 기계 코드목록 파일을 읽어 기계어 목록 테이블(inst_table)을
 *        생성하는 함수이다.
 * 매계 : 기계어 목록 파일
 * 반환 : 정상종료 = 0 , 에러 < 0
 * 주의 : 기계어 목록파일 형식은 자유롭게 구현한다. 예시는 다음과 같다.
 *
 *    ===============================================================================
 *           | 이름 | 형식 | 기계어 코드 | 오퍼랜드의 갯수 | NULL|
 *    ===============================================================================
 *
 * ----------------------------------------------------------------------------------
 */
int init_inst_file(char *inst_file)
{
    FILE *file;
    int errno;

    errno = 0;
    

    if (!(file = fopen(inst_file, "r")))
        errno = -1;

    for (int i = 0 ; i < MAX_INST ; i++)
    {
        inst_table[i] = calloc(1, sizeof(inst));
        if (4 != fscanf(file, "%s %d %x %c", inst_table[i]->str, &inst_table[i]->format, &inst_table[i]->ops, &inst_table[i]->op))
            break;
        else
            inst_index++;
    }

    fclose(file);
    return errno;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 어셈블리 할 소스코드를 읽어 소스코드 테이블(input_data)를 생성하는 함수이다.
 * 매계 : 어셈블리할 소스파일명
 * 반환 : 정상종료 = 0 , 에러 < 0
 * 주의 : 라인단위로 저장한다.
 *
 * ----------------------------------------------------------------------------------
 */
int init_input_file(char *input_file)
{
    FILE *file;
    int errno;
    char    buff[300];

    errno = 0;

    if (!(file = fopen(input_file, "r")))        errno = -1;

    buff_clear(buff);

    while(fgets(buff, sizeof(buff), file) != NULL) // 파일의 끝까지 읽는다.
    {
        if (buff[0] == '.') continue; // 주석일 경우 건너뛴다.
        //buff에 읽은 string을 input_table로 복사한다.
        input_data[line_num] = malloc(sizeof(char)*300);
        strcpy(input_data[line_num++], buff);
        buff_clear(buff);
    }

    fclose(file);
    return errno;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 소스 코드를 읽어와 토큰단위로 분석하고 토큰 테이블을 작성하는 함수이다.
 *        패스 1로 부터 호출된다.
 * 매계 : 파싱을 원하는 문자열
 * 반환 : 정상종료 = 0 , 에러 < 0
 * 주의 : my_assembler 프로그램에서는 라인단위로 토큰 및 오브젝트 관리를 하고 있다.
 * ----------------------------------------------------------------------------------
 */


int        token_parsing(char    *str)
{
    char        c;
    int            errno;
    int            operand_num;
    int            j;
    int            p;

    char        buff[4][100] = {'\0', };

    errno = 0;
    operand_num = 0;

    token_table[token_line] = malloc(sizeof(token));

    initialize_token(token_table[token_line]);

    sscanf(input_data[token_line], "%c", &c);

    if ((c == '\t') || (c == ' ')) // label 이 없을 때
        sscanf(input_data[token_line], "%s %s %s",buff[1], buff[2], buff[3]);
    else // label 이 있을 때
        sscanf(input_data[token_line], "%s %s %s %s",buff[0], buff[1], buff[2], buff[3]);

    if (buff[0][0] != '\0') // label 이 있을 때 label 저장
    {
        token_table[token_line]->label = calloc(strlen(buff[0]), sizeof(char));
        strcpy(token_table[token_line]->label, buff[0]);
    }

    ////////////////////////////////
    // operator 저장
    token_table[token_line]->operator = calloc(strlen(buff[1]), sizeof(char));
    strcpy(token_table[token_line]->operator, buff[1]);


    ///////////////////////////////
    // operand 저장
    //operator 의 foramt에 따른 operand 의 갯수 파악
    operand_num = search_operand_num(buff[1], buff[2]);
    token_table[token_line]->operand_num = operand_num;

    //operand의 갯수만큼 operand 저장
    j = 0;
    for (int i = 0 ; i < operand_num ; i++)
    {
        p = 0;
        while(buff[2][j] != ',' && buff[2][j] != '\0' && buff[2][j] != '\t')
            token_table[token_line]->operand[i][p++] = buff[2][j++];
        token_table[token_line]->operand[i][p] = '\0';

        if (buff[2][j] == ',') j++;
    }

    ////////////////////////////////////////////
    // nipxbpe 저장
    if (token_table[token_line]->operator[0] == '+')
    {
        token_table[token_line]->nixbpe |= 1;
        //operator 앞 + 삭제
        j = -1;
        while (token_table[token_line]->operator[++j] != '\0')
            token_table[token_line]->operator[j] = token_table[token_line]->operator[j+1];
    }
    if (token_table[token_line]->operand[0][0] == '#')
        token_table[token_line]->nixbpe |= 1 << 4;
    else if (token_table[token_line]->operand[0][0] == '@')
        token_table[token_line]->nixbpe |= 1 << 5;
    else
        token_table[token_line]->nixbpe |= 3 << 4;

    //X레지스터 참조할때
    if (token_table[token_line]->operand[1] != NULL && strcmp(token_table[token_line]->operand[1], "X") == 0)
        token_table[token_line]->nixbpe |= 1 << 3;

    // 기본 세팅은 pc relative 로
    token_table[token_line]->nixbpe |=  1 << 1;

    // RUSB 일때 pc relative 지워줌
    if (token_table[token_line]->operand[0][0] == '#'||
        (strcmp(token_table[token_line]->operator, "RSUB") == 0)||
        ((token_table[token_line]->nixbpe & 1) > 0 ))
            token_table[token_line]->nixbpe &= (~(1 << 1));

    token_line++;

    return errno;
}


/* ----------------------------------------------------------------------------------
 * 설명 : 입력 문자열이 기계어 코드인지를 검사하는 함수이다.
 * 매계 : 토큰 단위로 구분된 문자열
 * 반환 : 정상종료 = 기계어 테이블 인덱스, 에러 < 0
 * 주의 :
 *
 * ----------------------------------------------------------------------------------
 */
/*
int search_opcode(char *str)
{
    for (int i = 0 ; i < inst_index ; i++)
    {
        if (strcmp(inst_table[i]->str, str ) == 0) //inst_table내 str과 일치하는 string이 있을때
            return i; // 기계어 테이블 인덱스를 반환한다.
    }
    return -1;
}
*/
int     search_opcode(char *str)
{
    int i = -1;
    if (str[0] == '+') //명령어가 +로 시작될때, 정상적인 search를 위해서 +를 제외한 명령어로 다시 저장한다.
    {
        while(str[++i] != '\0')
            str[i] = str[i+1];
    }

    if (strncmp(str, "CSECT", 5) == 0)
    {
        return 24;
    }
    if (strncmp(str, "WORD", 4) == 0)
    {
        return 29;
    }
    if (strncmp(str, "LTORG", 5) == 0)
    {
        return 26;
    }

    for (int i = 0 ; i < inst_index ; i++)
    {
        if (strcmp(inst_table[i]->str, str ) == 0) //inst_table내 str과 일치하는 string이 있을때
            return i; // 기계어 테이블 인덱스를 반환한다.
    }
    return -1;
}

/* ----------------------------------------------------------------------------------
* 설명 : 어셈블리 코드를 위한 패스1과정을 수행하는 함수이다.
*           패스1에서는..
*           1. 프로그램 소스를 스캔하여 해당하는 토큰단위로 분리하여 프로그램 라인별 토큰
*           테이블을 생성한다.
*
* 매계 : 없음
* 반환 : 정상 종료 = 0 , 에러 = < 0
* 주의 : 현재 초기 버전에서는 에러에 대한 검사를 하지 않고 넘어간 상태이다.
*      따라서 에러에 대한 검사 루틴을 추가해야 한다.
*
* -----------------------------------------------------------------------------------
*/
static int assem_pass1(void)
{
    int            errno = 0;

    for (int i = 0 ; i < line_num ; i++)
        errno = token_parsing(input_data[i]);  // 한줄씩 parsing하고, 에러가 있는지 매회 검사한다.

    return errno;
}

/* ----------------------------------------------------------------------------------
* 설명 : 입력된 문자열의 이름을 가진 파일에 프로그램의 결과를 저장하는 함수이다.
*        여기서 출력되는 내용은 명령어 옆에 OPCODE가 기록된 표(과제 3번) 이다.
* 매계 : 생성할 오브젝트 파일명
* 반환 : 없음
* 주의 : 만약 인자로 NULL값이 들어온다면 프로그램의 결과를 표준출력으로 보내어
*        화면에 출력해준다.
*        또한 과제 4번에서만 쓰이는 함수이므로 이후의 프로젝트에서는 사용되지 않는다.
* -----------------------------------------------------------------------------------
*/
// void make_opcode_output(char *file_name)
// {
//     /* add your code here */

// }

/* ----------------------------------------------------------------------------------
* 설명 : 입력된 문자열의 이름을 가진 파일에 프로그램의 결과를 저장하는 함수이다.
*        여기서 출력되는 내용은 SYMBOL별 주소값이 저장된 TABLE이다.
* 매계 : 생성할 오브젝트 파일명
* 반환 : 없음
* 주의 : 만약 인자로 NULL값이 들어온다면 프로그램의 결과를 표준출력으로 보내어
*        화면에 출력해준다.
*
* -----------------------------------------------------------------------------------
*/

void make_symtab_output(char *file_name)
{
    FILE    *fp;

    fp = fopen(file_name, "w");

    //모든 token들에게 주솟값 부여
    set_tokens_addr();

    // symbol과 주솟값 기록
    for (int i = 0 ; i < token_line ; i++)
    {
        if (token_table[i]->label != NULL)
        {
            sym_table[sym_line].symbol = malloc(sizeof(char) * strlen(token_table[i]->label));
            strcpy(sym_table[sym_line].symbol, token_table[i]->label);
            sym_table[sym_line].addr = token_table[i]->addr;
            fprintf(fp, "%s  %x\n", token_table[i]->label, token_table[i]->addr);
            sym_line++;
        }
    }
    fclose(fp);
}

/* ----------------------------------------------------------------------------------
* 설명 : 입력된 문자열의 이름을 가진 파일에 프로그램의 결과를 저장하는 함수이다.
*        여기서 출력되는 내용은 LITERAL별 주소값이 저장된 TABLE이다.
* 매계 : 생성할 오브젝트 파일명
* 반환 : 없음
* 주의 : 만약 인자로 NULL값이 들어온다면 프로그램의 결과를 표준출력으로 보내어
*        화면에 출력해준다.
*
* -----------------------------------------------------------------------------------
*/

void make_literaltab_output(char *file_name)
{
    FILE    *fp;
    char    buff[10];
    int        buff_ptr;
    int        lit_ptr;


    fp = fopen(file_name, "w");

    for (int i = 0 ; i < token_line ; i++)
    {
        if (token_table[i]->operand[0][0] == '=')
        {
            if ( (literal_line != 0) && strcmp(token_table[i]->operand[0], literal_table[literal_line - 1].literal) == 0)
                continue;
            for (int j = i ; j < token_line ; j++)
            {
                if (strcmp(token_table[j]->operator, "LTORG") == 0 || strcmp(token_table[j]->operator, "END") == 0)
                {
                    literal_table[literal_line].literal = malloc(sizeof(char) * (strlen(token_table[i]->operand[0]) + 1));
                    strcpy(literal_table[literal_line].literal, token_table[i]->operand[0]);
                    literal_table[literal_line].addr = token_table[j]->addr;

                    lit_ptr = 3;
                    buff_ptr = 0;
                    buff_clear(buff);
                    while (literal_table[literal_line].literal[lit_ptr] != '\'')
                        buff[buff_ptr++] = literal_table[literal_line].literal[lit_ptr++];
                    buff[buff_ptr] = '\0';

                    strcpy(token_table[j]->operand[0], "");
                    strcpy(token_table[j]->operand[0], buff);

                    fprintf(fp, "%s\t%x\n", buff, literal_table[literal_line].addr);

                    literal_line++;

                    break;
                }
            }
        }
    }

    fclose(fp);
}

/* ----------------------------------------------------------------------------------
* 설명 : 어셈블리 코드를 기계어 코드로 바꾸기 위한 패스2 과정을 수행하는 함수이다.
*           패스 2에서는 프로그램을 기계어로 바꾸는 작업은 라인 단위로 수행된다.
*           다음과 같은 작업이 수행되어 진다.
*           1. 실제로 해당 어셈블리 명령어를 기계어로 바꾸는 작업을 수행한다.
* 매계 : 없음
* 반환 : 정상종료 = 0, 에러발생 = < 0
* 주의 :
* -----------------------------------------------------------------------------------
*/

static int assem_pass2(void)
{
    int                 format;
    int                 is_operation_num;
    unsigned long       object_code;

    for (int i = 0 ; i < token_line ; i++)
    {
        is_operation_num = is_operation(token_table[i]->operator);

        if (is_operation_num > 0) // object code를 생성하는 명령어일 경우
        {
            format = search_format(*token_table[i]);
            object_code = 0;

            operation_table[operation_line].format = format;

            if (format == 1) // 1형식일 경우
            {
                object_code |= set_operation_opcode(token_table[i], format);
                operation_table[operation_line].res = object_code;
                token_table[i]->cur_operation = &operation_table[operation_line++];
                continue;
            }
            else if (format == 2) // 2형식일 경우
            {
                object_code |= set_operation_opcode(token_table[i], format);
                object_code |= set_registers(token_table[i]);
                operation_table[operation_line].res = object_code;
                token_table[i]->cur_operation = &operation_table[operation_line++];

                continue;
            }
            else if (format == 3) // 3형식일 경우
            {
                object_code |= set_operation_opcode(token_table[i], format);
                object_code |= set_operation_nixbpe(token_table[i]);
                object_code |= set_operation_relative(i, token_table[i]);
                operation_table[operation_line].res = object_code;
                token_table[i]->cur_operation = &operation_table[operation_line++];
                continue;
            }
            else    return -1; // 에러 발생

        }
        else if (is_operation_num == 0) // object_code를 생성하지 않는 명령어일경우
            continue;
        else if (is_operation_num < 0) //LTORG를 만났을 때
        {
            operation_table[operation_line].format = 0;
            object_code = 0;

            if (strcmp(token_table[i]->operand[0], "EOF") == 0) object_code = 0x454f46;
            if (strcmp(token_table[i]->operand[0], "05") == 0) object_code = 0x05;

            operation_table[operation_line].res = object_code;
            token_table[i]->cur_operation = &operation_table[operation_line++];

        }
        else
            return -1; // 오류 발생
    }
    return 0;
}

/* ----------------------------------------------------------------------------------
* 설명 : 입력된 문자열의 이름을 가진 파일에 프로그램의 결과를 저장하는 함수이다.
*        여기서 출력되는 내용은 object code (프로젝트 1번) 이다.
* 매계 : 생성할 오브젝트 파일명
* 반환 : 없음
* 주의 : 만약 인자로 NULL값이 들어온다면 프로그램의 결과를 표준출력으로 보내어
*        화면에 출력해준다.
*
* -----------------------------------------------------------------------------------
*/

void make_objectcode_output(char *file_name)
{

    FILE        *fp;

    fp = fopen(file_name , "w");

    for (int i =0 ; i < operation_line ; i++)
        fprintf(fp, "%lx\n", operation_table[i].res);

    fclose(fp);

/*
    if (file_name == NULL)
    {



    }
    else
    {
        FILE    *fp;
        int     t_end_idx;
        token   *tmp;


        fp = fopen(file_name, "w");


        for (int i = 0 ; i < token_line ; i++)
        {
            tmp = token_table[i];
            locctr = token_table[i]->addr;
            t_end_idx = 0;

            if ( i == 0 || (tmp->addr == 0 && token_table[i-1]->addr != 0)) // 프로그램 시작
            {
                fputc('H', fp);
                fprintf(fp, "%-6s", tmp->label);
                fprintf(fp, "%6s%06x\n", "000000",program_len[program_cnt++]);
                continue;
            }

            if (strcmp(tmp->operator, "EXTDEF") == 0) // D 레코드
                {
                    fputc('D', fp);
                    for (int j = 0; j < MAX_OPERAND; j++)
                    {
                        if (tmp->operand[j] == NULL)        break;

                        fprintf(fp, "%6s", tmp->operand[j]);
                        fprintf(fp, "%06x", ret_symbol_addr(tmp->operand[j]));
                    }
                    fputc('\n', fp);
                    continue;
                }
            if (strcmp(tmp->operator, "EXTREF") == 0) // R 레코드
            {
                fputc('R', fp);
                for (int j = 0; j < MAX_OPERAND; ++j)
                {
                    if (tmp->operand[j] == NULL)        break;

                    fprintf(fp, "%6s", tmp->operand[j]);
                }
                fputc('\n', fp);
            }
            if (strcmp(tmp->operator, "END") == 0 || strcmp(tmp->operator, "CSECT") == 0 ) // 프로그램의 끝
            {

                fprintf(fp, "E\n" );
            }


            while(1)  // T 영역 기록
            {
                if (token_table[i]->addr == 0)      break;

                fputc('T', fp); // 코드영역
                t_end_idx = get_next_t_line(token_table[i]->addr);
                fprintf(fp, "%06x%02x", token_table[i]->addr, token_table[t_end_idx]->addr - token_table[i]->addr);

                for (int j = token_table[i]->addr ; j < t_end_idx && j < 51; j++)
                {
                    if(token_table[i]->cur_operation != NULL)
                        fprintf(fp, "%lx", token_table[i]->cur_operation->res);
                    i++;
                }
                fputc('\n', fp);

                if (i > 51 || strcmp(token_table[i]->operator, "END") == 0 || strcmp(token_table[i]->operator, "CSECT") == 0 )
                    break;
            }


            i = t_end_idx;
        }

    fclose(fp);
    }
*/
}



//////////////////////////////////////////////////////
////////////////////자체 제작 함수//////////////////////
/////////////////////////////////////////////////////



// * 설명 : string의 모든 요소를 '\0'로 초기화시킨다.
// * 매개 : 초기화시키고자 하는 string
// * 반환 : 모두 '\0'로 초기화된 string
// * 주의 :
void    buff_clear(char        *buff)
{
    int        buff_len;

    buff_len = (int)strlen(buff);

    for (int i = 0 ; i < buff_len ; i++)
        buff[i] = '\0';

    return;
}

// * 설명 : token 을 초기화시킨다.
// * 매개 : 초기화하고자 하는 token pointer
// * 반환 : void
// * 주의 : operand, comment의 경우 모든 string을 '\0'으로 초기화시키는게 아니라, 첫 글자를 초기화시킨다.
void        initialize_token(token    *tk)
{
    tk->label = NULL;
    tk->operator = NULL;
    buff_clear(tk->operand[0]);
    buff_clear(tk->operand[1]);
    buff_clear(tk->comment);
    tk->nixbpe = 0;
}



// * 설명 : operator를 판별한다.
// * 매개 : string
// * 반환 : operator - 0 반환, 아닌 경우 - 1 반환
// * 주의 :

int        find_not_operator(char    *str)
    {
        if(strcmp(str, "START") == 0
        || strcmp(str, "EXTDEF") == 0
        || strcmp(str, "EXTREF") == 0
        || strcmp(str, "RESW") == 0
        || strcmp(str, "RESB") == 0
        || strcmp(str, "EQU") == 0
        || strcmp(str, "CSECT") == 0
        || strcmp(str, "START") == 0
        || strcmp(str, "BYTE") == 0
        || strcmp(str, "WORD") == 0
        || strcmp(str, "END") == 0)
        {
            return 1;
        }

        return 0;
    }

// * 설명 : operand 의 갯수를 리턴한다.
// * 매개 : operator string, operand string
// * 반환 : operand의 갯수
// * 주의 :
int        search_operand_num(char        *str, char    *operand_str)
{
    int     operand_cnt = 1;

    for (int i = 0 ; i < inst_index ; i++)
    {
        if (strcmp(str, "EXTREF") == 0 || strcmp(str, "EXTDEF") == 0)
        {
            for (int j = 0 ; j < strlen(operand_str) ; j++)
                if (operand_str[j] == ',') operand_cnt++;

            return operand_cnt;
        }
        if(find_not_operator(str))
            return 1;
        else if (strcmp(inst_table[i]->str, str) == 0)
            return inst_table[i]->op - '0';
    }
    return 0;
}

// * 설명 : 해당 토큰의 format 을 리턴한다.
// * 매개 : 토큰
// * 반환 : 토큰에 해당하는 format (int)
// * 주의 :
int        search_format(token        tk)
{
    int        ret_num;


    // operator일 경우 format을 리턴
    for (int i = 0 ; i < inst_index ; i++)
    {
        if (strcmp(tk.operator, inst_table[i]->str) == 0)
            return inst_table[i]->format;
    }

    // operator가 아닌 경우 필요한 바이트 수만큼을 리턴
    if (strcmp(tk.operator, "BYTE")==0 || strcmp(tk.operator, "RESB")==0)
    {
        if (tk.operand[0][0] == 'C')
            return ((int)strlen(tk.operand[0]) - 3);
        else if (tk.operand[0][0] == 'X')
            return 1;
        else
        {
            sscanf(tk.operand[0], "%d", &ret_num);
            return ret_num;
        }
    }
    else if (strcmp(tk.operator, "EXTDEF")==0 || strcmp(tk.operator, "EXTREF")==0)
        return 0;
    else if (strcmp(tk.operator, "WORD")==0 || strcmp(tk.operator, "RESW")==0 || strcmp(tk.operator, "LTORG")==0)
        return 3;
    else
        return 0;

}

// * 설명 : 각 토큰에 주솟값을 저장한다.
// * 매개 : 주솟값을 저장하고자 하는 토큰
// * 반환 : -
void    set_tokens_addr()
{
    int        cur_addr;

    cur_addr = 0;

    for (int i = 0 ; i < token_line ; i++)
    {
        token_table[i]->addr_need_byte = search_format(*token_table[i]);
        if (token_table[i]->nixbpe & 1 ) // Extended일때 4바이트가 필요하다.
            token_table[i]->addr_need_byte = 4;
    }

    if (strcpy(token_table[0]->operator, "START") == 0) // 시작주소 설정
    {
        sscanf(token_table[0]->operand[0], "%d", &cur_addr);
        token_table[0]->addr = cur_addr;
    }

    for (int i = 0 ; i < token_line-1 ; i++)
    {
        if (strcmp(token_table[i]->operator, "CSECT")==0)
            cur_addr = 0;
        token_table[i+1]->addr = cur_addr + token_table[i]->addr_need_byte;
        cur_addr = token_table[i+1]->addr;
    }
}

// * 설명 : operation을 갖는 명령어인지 판별한다.
// * 매개 : 판별하고자 하는 operator  string
// * 반환 : operation을 가지면 > 0 / 가지지않으면 0 / 프로그램의 끝이면 < 0
int            is_operation(char    *str)
{
    if (strcmp(str, "START") == 0 ||
    strcmp(str, "EXTDEF") == 0 ||
    strcmp(str, "EXTREF") == 0 ||
    strcmp(str, "RESW") == 0 ||
    strcmp(str, "RESB") == 0 ||
    strcmp(str, "EQU") == 0 ||
    strcmp(str, "CSECT") == 0)
        return 0;

    if (strcmp(str, "LTORG") == 0 ||
        strcmp(str, "END") == 0 )
        return -1;

    else
        return 1;

}



// * 설명 : 명령어에 opcode를 적절한 위치로 비트연산시켜서 리턴한다.
// * 매개 : 저장하고자 하는 token, format
// * 반환 : opcode가 적절한 위치로 비트연산된 명령어
unsigned long        set_operation_opcode(token    *_token, int format)
{
    unsigned int         opcode_buff = 0;
    unsigned long        res_buff = 0;
    unsigned long        extend_mask = 0b11111100000000000000000000000000;
    unsigned long        normal_mask = 0b00000000111111000000000000000000;

    for (int i = 0 ; i < inst_index ; i++)
    {
        if ((strcmp(_token->operator, inst_table[i]->str) == 0))
        {
            opcode_buff =inst_table[i]->ops;
            break;
        }
    }

    if (format == 1)
    {
        return opcode_buff;
    }

    if (format == 2)
    {
        res_buff |= opcode_buff << 8;
        return res_buff;
    }

    else // 3형식일때
    {
        if (_token->nixbpe & 1) // extend 일때
        {
            res_buff |= opcode_buff << 24;
            res_buff &= extend_mask;
            return res_buff;
        }
        else
        {
            res_buff |= opcode_buff << 16;
            res_buff &= normal_mask;
            return res_buff;
        }
    }
}



// * 설명 : 명령어에 nixbpe 적절한 위치로 비트연산시켜서 리턴한다.
// * 매개 : 저장하고자 하는 token, format
// * 반환 : nixbpe가 적절한 위치로 비트연산된 명령어
unsigned long        set_operation_nixbpe(token    *_token)
{
    unsigned long       res_buff = 0;

    if (_token->nixbpe & 1)
    {
        res_buff |= (_token->nixbpe << 20);
        return res_buff;
    }
    else
    {
        res_buff |= (_token->nixbpe << 12);
        return res_buff;
    }
}

// * 설명 : direct addressing 을 위해 trim 된 string을 리턴한다.
// * 매개 : Trim 하고자 하는 Operand
// * 반환 : 앞 뒤로 특수문자를 제거한 string을 리턴한다.
char        *trim_for_direct_addr(char     *str)
{
    int        len;
    char    *res_char;

    len = (int)strlen(str);
    res_char = calloc(len, sizeof(char));


    if (len == 0 || len == 1)    return 0;

    for (int i = 0 ; i < len ; i++)
    {
        res_char[i] = str[i+1];
    }

    return (res_char);
}



// * 설명 : 주어진 operand 의 주소를 리턴한다.
// * 매개 : 주어진 operand 의 주소
// * 반환 : 주소를 찾고자하는 operand
int            search_target_addr(char        *operand)
{
    if (strcmp(operand, "A") == 0)
        return 0;
    else if (strcmp(operand, "X") == 0)
        return 1;
    else if (strcmp(operand, "L") == 0)
        return 2;
    else if (strcmp(operand, "T") == 0)
        return 3;
    else if (strcmp(operand, "S") == 0)
        return 4;
    else if (operand[0] == '#')
        return (atoi(trim_for_direct_addr(operand)));
    else if (operand[0] == '@')
        return 0;
    else if (operand[0] == '=')
    {
        operand = trim_for_direct_addr(operand);
        for (int i = 0 ; i < literal_line ; i++)
        {
            if (strcmp(trim_for_direct_addr(literal_table[i].literal), operand) == 0)
                return literal_table[i].addr;
        }
    }
    else // label 일때
    {
        for (int i = 0 ; i < sym_line ; i++)
        {
            if (strcmp(sym_table[i].symbol, operand) == 0)
            {
                return sym_table[i].addr;
            }
        }
    }
    return 0;
}

// * 설명 : 명령어에 relative addressing이 적절한 위치로 비트연산시켜서 리턴한다.
// * 매개 : 저장하고자 하는 token, format
// * 반환 : relative addressing가 적절한 위치로 비트연산된 명령어
unsigned long        set_operation_relative(int    index, token    *_token)
{
    unsigned int           target_addr;
    unsigned int           pc;
    int                    disp_buff;
    int                    disp_mask;
    unsigned long          res_buff = 0;


    disp_mask = 0b00000000000000000000111111111111;

    if ((_token->nixbpe & 1) > 0 )
        return 0;
    if (_token->nixbpe == 16) //immediate addressing일때
        return (atoi(trim_for_direct_addr(_token->operand[0])));
    else if (_token->nixbpe == 0x22) // direct addressing
        return 0;
    else
    {
        pc = (unsigned int)token_table[index+1]->addr;
        target_addr = search_target_addr(_token->operand[0]);
        disp_buff = target_addr - pc;
        disp_buff &= disp_mask;
        res_buff |= disp_buff;
        return res_buff;
    }
}


// * 설명 : 명령어에 register가 적절한 위치로 비트연산시켜서 리턴한다.
// * 매개 : 저장하고자 하는 token, format
// * 반환 : register가 적절한 위치로 비트연산된 명령어
unsigned long        set_registers(token    *_token)
{
    int                 reg1;
    int                 reg2;
    unsigned long       res_buff = 0;

    reg1 = search_target_addr(_token->operand[0]);
    reg2 = search_target_addr(_token->operand[1]);

    res_buff |= reg1 << 4;
    res_buff |= reg2;

    return res_buff;

}


// * 설명 : 찾고자하는 symbol이 sym_tab에서 가지는 인덱스를 찾는다.
// * 매개 : 찾고자하는 symbol
// * 반환 : 찾고자하는 symbol이 sym_tab에서 가지는 인덱스
int     ret_symbol_addr(char    *symbol)
{
    for (int i = 0 ; i < sym_line ; i++)
    {
        if (strcmp(symbol, sym_table[i].symbol) == 0)
            return sym_table[i].addr;
    }
    return 0;
}



///////////////////////////////////////////
///////////////확인용 출력 함수 //////////////
//////////////////////////////////////////


// inst_table 출력
void    print_inst_table()
{
    for (int i = 0 ; i < inst_index ; i++)
        printf("str:%10s\top:%c\tformat:%d\tops:%x\n", inst_table[i]->str, inst_table[i]->op, inst_table[i]->format, inst_table[i]->ops);

    return;
}

// sym_table 출력
void    print_sym_table()
{
    for (int i = 0 ; i < sym_line ; i++)
        printf("%s  %x\n", sym_table[i].symbol, sym_table[i].addr);
}


// token_table 출력
void    print_token_table()
{
    for (int i = 0 ; i < token_line ; i++)
    {
        printf("%s\t", token_table[i]->label);
        printf("%s\t", token_table[i]->operator);
        printf("%s\t", token_table[i]->operand[0]);
        printf("%s\t", token_table[i]->operand[1]);
        printf("%c\t", token_table[i]->nixbpe);
        printf("addr:%x\n", token_table[i]->addr);
    }

    return;
}

// operation_table 출력
void    print_operation_table()
{
    for (int i = 0 ; i < operation_line ; i++)
        printf("%lx\n", operation_table[i].res);

    return ;
}

