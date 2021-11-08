
/*
 * my_assembler 함수를 위한 변수 선언 및 매크로를 담고 있는 헤더 파일이다.
 *
 */
#define MAX_INST 256
#define MAX_LINES 5000
#define MAX_OPERAND 3


/*
 * operation을 저장하는 구조체이다.
 * 모든 operation을 저장한다.
 */

struct operation_unit
{
    int                format; // 형식
    unsigned long       res; // 최종명령어
};
typedef struct operation_unit operation;
operation operation_table[MAX_LINES];
static int operation_line;



/*
 * instruction 목록 파일로 부터 정보를 받아와서 생성하는 구조체 변수이다.
 * 구조는 각자의 instruction set의 양식에 맞춰 직접 구현하되
 * 라인 별로 하나의 instruction을 저장한다.
 */
struct inst_unit
{
    char str[10];
    unsigned char op;
    int format;
    int ops;
};

// instruction의 정보를 가진 구조체를 관리하는 테이블 생성
typedef struct inst_unit inst;
inst *inst_table[MAX_INST];
int inst_index;

/*
 * 어셈블리 할 소스코드를 입력받는 테이블이다. 라인 단위로 관리할 수 있다.
 */
char *input_data[MAX_LINES];
static int line_num;

/*
 * 어셈블리 할 소스코드를 토큰단위로 관리하기 위한 구조체 변수이다.
 * operator는 renaming을 허용한다.
 * nixbpe는 8bit 중 하위 6개의 bit를 이용하여 n,i,x,b,p,e를 표시한다.
 */
struct token_unit
{
    char *label;
    char *operator;
    char operand[MAX_OPERAND][20];
    char comment[100];
    char nixbpe;
    // 추가된 변수
    int     addr_need_byte;
    int     addr;
    int     operand_num;
    operation   *cur_operation;
};

typedef struct token_unit token;
token *token_table[MAX_LINES];
static int token_line;

/*
 * 심볼을 관리하는 구조체이다.
 * 심볼 테이블은 심볼 이름, 심볼의 위치로 구성된다.
 */
struct symbol_unit
{
    char *symbol;
    int addr;
};
typedef struct symbol_unit symbol;
symbol sym_table[MAX_LINES];
static int        sym_line;

/*
* 리터럴을 관리하는 구조체이다.
* 리터럴 테이블은 리터럴의 이름, 리터럴의 위치로 구성된다.
* 추후 프로젝트에서 사용된다.
*/
struct literal_unit
{
    char *literal;
    int addr;
    int checked;
};

typedef struct literal_unit literal;
literal literal_table[MAX_LINES];
static int        literal_line;


static int locctr;
//--------------

static char *input_file;
static char *output_file;
int init_my_assembler(void);
int init_inst_file(char *inst_file);
int init_input_file(char *input_file);
int token_parsing(char *str);
int search_opcode(char *str);
static int assem_pass1(void);
void make_opcode_output(char *file_name);

void make_symtab_output(char *file_name);
void make_literaltab_output(char *file_name);
static int assem_pass2(void);
void make_objectcode_output(char *file_name);





//===============================================
void                  initialize_token(token    *tk);
void                  buff_clear(char        *buff);
int                   find_not_operator(char    *str);
int                    search_operand_num(char        *str, char    *operand_str);
int                    search_format(token        tk);
void                   set_tokens_addr(void);
int                    is_operation(char    *str);
unsigned long          set_operation_opcode(token    *_token, int format);
unsigned long          set_operation_nixbpe(token    *_token);
char                   *trim_for_direct_addr(char     *str);
int                    search_target_addr(char        *operand);
unsigned long          set_operation_relative(int    index, token    *_token);
unsigned long          set_registers(token    *_token);
int                    ret_symbol_addr(char    *symbol);
void                   print_inst_table(void);
void                   print_sym_table(void);
void                   print_token_table(void);
void                   print_operation_table(void);
