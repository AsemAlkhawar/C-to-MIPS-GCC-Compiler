/*
 * File: parser-driver.c
 * Author: Saumya Debray
 * Purpose: The driver code for the CSC 453 project.  It uses command-line
 *          arguments to control whether semantic checks and/or code 
 *          generation is carried out.
 */

#include <stdio.h>
#include <string.h>
#include "scanner.h"
#include "gen_code.h"

extern int parse();

int chk_decl_flag = 0;      /* set to 1 to do semantic checking */
int print_ast_flag = 0;     /* set to 1 to print out the AST */
int gen_code_flag = 0;      /* set to 1 to generate code */
int gen_3AC_flag = 0;

/*
 * parse_args() -- parse command-line arguments and set flags appropriately
 * to control the actions performed by the compiler.  The arguments are:
 *
 *    --chk_decl     : to check legality of declarations
 *    --print_ast    : to print out the AST of each function
 *    --gen_code     : to generate code
 */
void parse_args(int argc, char *argv[]) {
  int i;
  for (i = 0; i < argc; i++) {
    if (argv[i][0] == '-') {
      if (strcmp(argv[i], "--chk_decl") == 0) {
	    chk_decl_flag = 1;
      }
      else if (strcmp(argv[i], "--print_ast") == 0) {
	    print_ast_flag = 1;
      }
      else if (strcmp(argv[i], "--gen_code") == 0) {
	    gen_code_flag = 1;
      }
      else if (strcmp(argv[i], "--gen_instr") == 0) {
        gen_3AC_flag = 1;
      }
      else {
	fprintf(stderr, "Unrecognized option: %s\n", argv[i]);
      }
    }
  }
}

int main(int argc, char *argv[]) {
  int error_code;
  parse_args(argc, argv);

  if (gen_code_flag) {
      add_to_symbol_table("println", FUNCTION, GLOBAL);
      update_function_arg_count("println", 1);
      FILE* mipsFile = fopen("./MIPS_CODE.s", "w");
      printFileContents();
      appendFileContentsToFile(mipsFile);
      fclose(mipsFile);

      FILE* file = fopen("./instr.txt", "w");  // Open the file for writing, overwriting existing contents
      fclose(file);  // Immediately close the file, leaving it empty
  }


  
  error_code = parse();
  if (gen_code_flag) {
      FILE *temp = fopen("./MIPS_CODE.s", "a");
      generateDataSection(temp);
      fclose(temp);
  }

  return error_code;
}





//int collatz_line(int val) {
//    if (val % 2 == 1) {  // Check if the value is odd
//        printf("%d\n", val);
//        return val;
//    }
//
//    printf("%d ", val); // Print the initial value if it's not odd
//    int cur = val;
//    while (cur % 2 == 0) {
//        cur /= 2;
//        printf("%d ", cur);  // Print each value after dividing by 2
//    }
//
//    printf("\n");
//    return cur; // Return the last value obtained
//}


//void collatz(int val) {
//    int cur = val;
//    int calls = 0;
//
//    cur = collatz_line(cur);
//
//    while (cur != 1) {
//        cur = 3 * cur + 1;
//        cur = collatz_line(cur);
//        calls++;
//    }
//
//    printf("collatz(%d) completed after %d calls to collatz_line().\n", val, calls);
//    printf("\n");
//}
//
//int main() {
//    collatz_line(12);
//    collatz(123);
//    return 0;
//}

//char getNextLetter(int pos) {
//    // ASCII value of 'A' is 65. We use 'pos' to find corresponding character.
//    if (pos >= 0 && pos < 26) { // There are 26 letters in the English alphabet
//        return 'A' + pos;       // Map position to corresponding letter
//    }
//    else {
//        return '\0';            // Return '\0' if position is out of alphabet range
//    }
//}
//
//int letterTree(int step)
//{
//    int count, pos, i;
//    int letter;
//
//    count = 0;
//    pos = 0;
//    letter = getNextLetter(pos); // Initialize c before entering the loop
//
//    // Use a while loop that continues as long as c is not the zero character
//    while (letter != '\0')  // '\0' represents *ZERO* character
//    {
//        i = 0;  // Initialize the loop control variable for the while loop
//        // Equivalent to the for loop using a while loop
//        while (i < count)
//        {
//            printf("%c", letter);  // use syscall 11 for MIPS, printf for C
//            printc(letter);
//            i = i + 1;
//        }
//        println();
//
//        count++;
//        pos += step;
//        letter = getNextLetter(pos); // Update c at the end of the loop
//    }
//
//    return pos;
//}
//
//int main() {
//    letterTree(1);
//    return 1;
//}