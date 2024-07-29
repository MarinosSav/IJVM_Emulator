#include <ijvm.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <frame.h>

/*
   ------------------------
   | Marinos Savva msa287 |
   ------------------------
*/

/*|||||||||||||||||||||||||||| VARIABLES ||||||||||||||||||||||||||||||||||||*/

//IO variables
FILE* output;
FILE* input;

//buffer variables
byte_t* data_buffer;
byte_t* text_buffer;
char keyboard_buffer;
word_t txt_size, const_size;

//control variables
int pc;
bool done;
int testing = 0;
bool wide;

//frame variables
frame* frm;
frame** frm_store;
int top_frame_store;

/*|||||||||||||||||||||||||| UTILITY FUNCTIONS |||||||||||||||||||||||||||||||*/


void init_frm_store(void)
{
  /* creates an array of frames */
  frm = new_frame();
  frm_store = malloc(sizeof (*frm) * 100);
  top_frame_store = 0;
}

int16_t arr_to_int(byte_t* bytes)
{
  /* converts an array of 4 bytes to an int16 */
  uint16_t temp =  bytes[0] + (bytes[1] << 8) + (bytes[2] << 16) + (bytes[3] << 24);
  return (temp>>8) | (temp<<8);
}

word_t swap_word(word_t num)
{
  /* switches endianess for a word */
 return ((num>>24)&0xff) | ((num<<8)&0xff0000) | ((num>>8)&0xff00) | ((num<<24)&0xff000000);
}

word_t get_word(FILE* fp)
{
  /* gets next word from file */
  word_t word;
  fread(&word, sizeof(word_t), 1, fp);

  return swap_word(word);
}

void branch(void)
{
  /* branches to the pc indicated by next 2 bytes */
  byte_t* temp = malloc(sizeof(byte_t) * 2);
  temp[0] = text_buffer[pc + 1];
  temp[1] = text_buffer[pc + 2];
  pc = pc + arr_to_int(temp);
  free(temp);
}

int get_short(int pc)
{
  /* creates a short out of 2 bytes*/
  byte_t* temp = malloc(sizeof(byte_t) * 2);
  temp[0] = text_buffer[pc + 1];
  temp[1] = text_buffer[pc + 2];
  int out = arr_to_int(temp);
  free(temp);
  return out;
}

/*||||||||||||||||||||||||||||| IJVM FUNCTIONS |||||||||||||||||||||||||||||||*/

int init_ijvm(char *binary_file)
{
  /* initiates IJVM simulation */
  FILE* fp;
  fp = fopen(binary_file, "rb");
  set_output(stdout);
  set_input(stdin);
  pc = 0;
  wide = false;
  done = false;
  testing++;

  init_frm_store();

  word_t magic = get_word(fp);
  printf("%x\n",magic );
  if (magic != MAGIC_NUMBER){
    printf("%s\n", "This is not an IJVM");
    exit(0);
  }
  get_word(fp);
  const_size = get_word(fp);

  data_buffer = malloc(const_size + 1);
  for(word_t i = 0; i < const_size; i++)
  {
    fread(&data_buffer[i], sizeof(byte_t), 1, fp);
  }

  get_word(fp);
  txt_size = get_word(fp);

  text_buffer = malloc(txt_size + 1);
  for(word_t i = 0; i < txt_size; i++)
  {
    fread(&text_buffer[i], sizeof(byte_t), 1, fp);
  }

  fclose(fp);
  return 0;
}

void destroy_ijvm()
{
  /* destroys IJVM simulation */
  free(text_buffer);
  free(data_buffer);
  free(frm_store);
  destroy_frame(frm);
  pc = 0;
}

void run()
{
  /* steps while program is still going*/
  while(!done)
  {
    step();
  }
}

void set_input(FILE *fp)
{
  /* sets the input source to the file pointer */
  input = fp;
}

void set_output(FILE *fp)
{
  /* sets the output destination to the file pointer */
  output = fp;
}

word_t tos(void)
{
  /* returns the top most element of the current frames stack */
  return top(frm);
}

word_t *get_stack(void)
{
  /* gets a pointer to the stack of the current frame */
  return get_elements(frm);
}

int stack_size(void)
{
  /* gets the current frames stack size */
  return get_frm_size(frm);
}

byte_t *get_text(void)
{
  /* gets the text buffers contents */
  return text_buffer;
}

int text_size(void)
{
  /* gets the text buffers size */
  return txt_size;
}

int get_program_counter(void)
{
  /* gets the current program counter value */
  return pc;
}

word_t get_local_variable(int i)
{
  /* gets the local variable with index 'i' of the current frame */
  return get_locals(frm)[i];
}

word_t get_constant(int i)
{
  /* gets the value from the contstant pool with index 'i'  */
  byte_t* temp = malloc(sizeof(byte_t) * 2);
  int index = (((i + 1) * 4) - 1);
  for (int j = 0; j < 4; j++)
  {
    temp[j] = data_buffer[index + j];
  }
  word_t out = temp[0] + (temp[1] << 8) + (temp[2] << 16) + (temp[3] << 24);
  free(temp);
  return out;
}

bool finished(void)
{
  /* gets the simulations current state */
  return done;
}

byte_t get_instruction(void)
{
  /* gets the next instruction to be executed */
  return text_buffer[pc];
}

bool step(void)
{
  /* executes the next instruction in line */
  word_t temp1, temp2;
  char* opcode;

  switch (text_buffer[pc])
    {
      case OP_BIPUSH:
      {
        opcode = "BIPUSH";
        pc++;
        push(frm, (int8_t) text_buffer[pc]);
        pc++;
        break;
      }
      case OP_DUP:
      {
        opcode = "DUP";
        push(frm, tos());
        pc++;
        break;
      }
      case OP_ERR:
      {
        opcode = "ERR";
        done = true;
        pc++;
        return false;
      }
      case OP_GOTO:
      {
        opcode = "GOTO";
        branch();
        break;
      }
      case OP_HALT:
      {
        opcode = "HALT";
        done = true;
        return false;
      }
      case OP_IADD:
      {
        opcode = "IADD";
        temp1 = pop(frm);
        temp2 = pop(frm);
        push(frm, temp1 + temp2);
        pc++;
        break;
      }
      case OP_IAND:
      {
        opcode = "IAND";
        temp1 = pop(frm);
        temp2 = pop(frm);
        push(frm, temp1 & temp2);
        pc++;
        break;
      }
      case OP_IFEQ:
      {
        opcode = "IFEQ";
        temp1 = pop(frm);
        if (temp1 == 0)
          branch();
        else
          pc += 3;
        break;
      }
      case OP_IFLT:
      {
        opcode = "IFLT";
        temp1 = pop(frm);
        if (temp1 < 0)
          branch();
        else
          pc += 3;
        break;
      }
      case OP_ICMPEQ:
      {
        opcode = "ICMPEQ";
        temp1 = pop(frm);
        temp2 = pop(frm);
        if (temp1 == temp2)
          branch();
        else
          pc += 3;
        break;
      }
      case OP_IINC:
      {
        opcode = "IINC";
        pc++;
        set_local(frm, text_buffer[pc], get_locals(frm)[text_buffer[pc]] + (int8_t) text_buffer[pc + 1]);
        pc += 2;
        break;
      }
      case OP_ILOAD:
      {
        opcode = "ILOAD";
        if (wide)
        {
          push(frm, get_locals(frm)[(int16_t) get_short(pc)]);
          pc += 3;
          wide = false;
        }
        else
        {
          pc++;
          push(frm, get_locals(frm)[(int8_t) text_buffer[pc]]);
          pc++;
        }
        break;
      }
      case OP_IN:
      {
        opcode = "IN";
        int result = getc(input);
        if (result < 0)
          push(frm, 0);
        else
          push(frm, result);
        pc++;
        break;
      }
      case OP_INVOKEVIRTUAL:
      {
        opcode = "INVOKE";
        frame* temp_frm = new_frame();
        int argpoint = (int8_t) get_constant(get_short(pc));

        set_local(temp_frm, 0, pc + 3);

        pc = argpoint - 1;
        int argnum = get_short(pc);

        pc += 5;
        //int temp_size = get_short(pc) + 2;

        for (int i = 1; i < argnum; i++)
        {
          set_local(temp_frm, argnum - i , pop(frm));
        }

        frm_store[top_frame_store] = frm;
        top_frame_store++;
        frm = temp_frm;
        frm_store[top_frame_store] = frm;
        break;
      }
      case OP_IOR:
      {
        opcode = "IOR";
        temp1 = pop(frm);
        temp2 = pop(frm);
        push(frm, temp1 | temp2);
        pc++;
        break;
      }
      case OP_IRETURN:
      {
        opcode = "IRETURN";
        frame* temp_frm = frm_store[top_frame_store];
        frm = frm_store[top_frame_store - 1];

        push(frm, top(temp_frm));
        pc = get_locals(temp_frm)[0];

        destroy_frame(temp_frm);
        top_frame_store--;
        break;
      }
      case OP_ISTORE:
      {
        opcode = "ISTORE";
        if (wide)
        {
          set_local(frm,  (uint16_t) get_short(pc), tos());
          pc += 3;
          wide = false;
        }
        else
        {
          pc++;
          set_local(frm, (int8_t) text_buffer[pc], tos());
          pc++;
        }
        pop(frm);
        break;
      }
      case OP_ISUB:
      {
        opcode = "ISUB";
        temp1 = pop(frm);
        temp2 = pop(frm);
        push(frm, temp2 - temp1);
        pc++;
        break;
      }
      case OP_LDC_W:
      {
        opcode = "LDC_W";
        push(frm, (int8_t) get_constant(get_short(pc)));
        pc += 3;
        break;
      }
      case OP_NOP:
      {
        opcode = "NOP";
        pc++;
        break;
      }
      case OP_OUT:
      {
        opcode = "OUT";
        fprintf(output, "%c", (char)pop(frm));
        pc++;
        break;
      }
      case OP_POP:
      {
        opcode = "POP";
        pop(frm);
        pc++;
        break;
      }
      case OP_SWAP:
      {
        opcode = "SWAP";
        temp1 = pop(frm);
        temp2 = pop(frm);
        push(frm, temp1);
        push(frm, temp2);
        pc++;
        break;
      }
      case OP_WIDE:
      {
        opcode = "WIDE";
        wide = true;
        pc++;
        step();
        break;
      }
      default:
      {
        pc++;
      }
    }
    printf("%s\n", opcode);
    return true;
}
