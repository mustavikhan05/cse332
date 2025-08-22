/*
 * MIPS Assembler : CS2610 Assembly Language Programming Lab Project
 *
 *  Created on: April 20, 2019
 *     Authors:	NILESH TIWARI CS17B022
 				VORA BRIJESH HARSHADBHAI CS17B031
 */

#include <bits/stdc++.h>
#include "mips.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "data_symbol_table.h"

using namespace std;

#define cout cout<<__FILE__<<":"<<__LINE__<<": "

// Using global variables is a horrible ides
// Do as I say, not as I do
unordered_map<string, int> label_address; // variable to store the address of label

int pc = 0;//program_counter >> 2

// set of all valid labels in input file
set<string> labels;

// map of all valid labels in input file with their addresses
map<string, int> LABEL;

// set of all valid labels of data section in input file
set<string> datalabels;

// map of all valid labels of data section in input file with their addresses
map<string, int> DATALABEL;

// line number in input file for two sections data and text
int section[2] = {-1,-1};

int address = 0;

// Variable to keep track of which section that is currently being processed
int current_section = -1;

// A symbol table used to generate code for the data section
DataSymbolTable data_section_symbol_table;

// A map to store a list of unresolved symbols in the data section. The key is the
// symbol string, and the value is a pointer to the variable in the symbol table
// which is supposed to contain the value of the symbol in the MIPS code.
unordered_map<string, void*> unresolved_symbols;

// this function determines the type of the valid line in the input file as invalid, instruction, .data, .text, labels
// return value mapping:
//  0 -> error
//  1 -> label
//  2 -> instruction
//  3 -> data section
//  4 -> text section
int string_type(string s, int* previous_string_type)
{
    cout << "Entered function: string_type" << endl;
    cout << "working with line: " << s << endl;
    cout << "section[0]: " << section[0] << endl;
    cout << "section[1]: " << section[1] << endl;
    cout << "pc: " << pc << endl;

	int i = 0;
	string s1 = "";
	while(s[i] != ' ')
	{
		s1 += s[i];
		i++;
	}

    cout << s1 << endl;

	if(s1 == ".data")
	{
		if(i == s.size()-1) {
            *previous_string_type = 3;
            return 3;
        }
		else { return 0; }
	}

	if(s1 == ".text")
	{
		if(i == s.size()-1) {
            *previous_string_type = 4;
            return 4;
        }
		else return 0;
	}

    // check if I received an instruction
	if(instruction.count(s1)>0)
	{
		return 2;
	}

	int idx = s1.size()-1;
    cout << endl;
    // the following variable is set to true if the current line that is being processed is of the following format:
    //  <string>:
    // i.e., it is a string followed by `:`
    // Checking for ascii code 95 allows labels to start with a preceding underscore: `_`
    const
    bool is_label_like = ((s1[0]>=65 && s1[0] <=90) || (s1[0]>=97 && s1[0] <=122) || s[0] == 95) && s1[idx] == ':';
	if(s1.size()>1 && is_label_like)
	{
        cout << "Processing \"" << s1 << "\" in \"" << s << "\" as label" << endl;
		string s2 = "";
		s2 = s1;
		s2.pop_back();
		if(instruction.count(s2)>0) // make sure the instruction is supported by the assembler
		{
            cout << endl;
			return 0;
		}
		else {
            cout << s1 << " is not an instruction" << endl;
            if (current_section == 3) return 1; // .data section cannot contain instructions
            return 1;
        }
	}

    cout << "s1[0]: |" << s1[0] << "|" << endl;
    if (s1[0] == 46) { // encountered period
        // I am assuming that the following is invalid in the text section
        // label:
        //  .word 42
        if (current_section == 3) {
            cout << "need to add something here" << endl;
            return 1;
        }
    }

	if(i+1 < s.size() && ((s1[0]>=65 && s1[0] <=90) || (s1[0]>=97 && s1[0] <=122) || s[0] == '_'))
	{
        cout << endl;
		if(s[i+1] == ':' && s[i+2] == ' ') {
            cout << endl;
			return 1;
        }
	}

    cout << endl;
	return 0;
}

//for processing instruction type1 syscall and nop
void type1(string opname, string operand, int operand_count, int line_count, int* valid, ofstream &file, ofstream &log, int write)
{
	if(operand_count == 0)
	{
		if(opname == "syscall")
		{
			printhex(pc*4, file);
			pc++;
            file<<"0000 0000 0000 0000 0000 0000 0000 0000"<<endl;

            printhex(pc*4, file);
            pc++;
			file<<"0000 0000 0000 0000 0000 0000 0000 1100"<<endl;
		}
		else if(opname == "nop")
		{
			printhex(pc*4, file);
			pc++;
			file<<"0000 0000 0000 0000 0000 0000 0000 0000"<<endl;
		}
	}
	else
	{
		*valid = 0;
		cout<<"syntax error at line "<<line_count<<endl;
		if(write == 1)
			log<<"syntax error at line "<<line_count<<endl;
		return;
	}
}

//for processing instruction type2 : op r1, r2, r3
void type2(string opname, string operand, int operand_count, int line_count, int* valid, ofstream &file, ofstream &log, int write)
{
	if(operand_count == 3)
	{
		int i = 0;
		string r1 = "";
		string r2 = "";
		string r3 = "";
		while(operand[i] != ' ')
		{
			r1 += operand[i];
			i++;
		}

		int j = i+1;
		while(operand[j] != ' ')
		{
			r2 += operand[j];
			j++;
		}

		int k = j+1;
		while(operand[k] != ' ')
		{
			r3 += operand[k];
			k++;
		}

		if(!(registers.count(r1)>0 && registers.count(r2)>0 && registers.count(r3)>0))
		{
            cout << "r1: " << r1 << endl;
            cout << "r2: " << r2 << endl;
            cout << "r3: " << r3 << endl;
            cout << "registers.count("<<r1<<"): "<< registers.count(r1) << endl;
            cout << "registers.count("<<r2<<"): "<< registers.count(r2) << endl;
            cout << "registers.count("<<r3<<"): "<< registers.count(r3) << endl;
			*valid = 0;
			cout<<"syntax error at line "<<line_count<<endl;
			if(write == 1)
				log<<"syntax error at line "<<line_count<<endl;
			return;
		}

		string op = "000000";
		string rs, rt, rd;
		string shamt = "00000";
		string funct;
		rs = reg_op[r2];
		rt = reg_op[r3];
		rd = reg_op[r1];
		funct = type2_op[opname];

		string result = "";
		result += op;
		result += rs;
		result += rt;
		result += rd;
		result += shamt;
		result += funct;

		string res_final = "";
		for(int id=0; id<result.size(); id++)
		{
			if( id!=0 && id%4 == 0)
			{
				res_final += " ";
			}

			res_final += result[id];
		}

		printhex(pc*4, file);
		pc++;
		file<<res_final<<endl;

	}
	else
	{
		*valid = 0;
		cout<<"syntax error at line "<<line_count<<endl;
		if(write == 1)
			log<<"syntax error at line "<<line_count<<endl;
		return;
	}

}

//for processing instruction type2 : op r1, r2, imm or label
void type3(string opname, string operand, int operand_count, int line_count, int* valid, ofstream &file, ofstream &log, int write)
{
	if(operand_count == 3)
	{
		int i = 0;
		string r1 = "";
		string r2 = "";
		string imm = "";
		while(operand[i] != ' ')
		{
			r1 += operand[i];
			i++;
		}

		int j = i+1;
		while(operand[j] != ' ')
		{
			r2 += operand[j];
			j++;
		}

		int k = j+1;
		while(operand[k] != ' ')
		{
			imm += operand[k];
			k++;
		}

		int negative = 0;
		int int_flag = 1;
		bool is_label = false;

		if(imm[0]=='-' && imm.size()==1)
		{
			*valid = 0;
			cout<<"syntax error at line "<<line_count<<": not valid immediate value"<<endl;
			if(write == 1)
				log<<"syntax error at line "<<line_count<<": not valid immediate value"<<endl;
			return;
		}

		if(imm[0] == '-')
		{
			negative = 1;
		}

		// Check if immediate is a number or a label
		if(negative == 0 && !(imm[0] >= '0' && imm[0] <= '9'))
		{
			// Could be a label, check if it exists in datalabels
			if(datalabels.count(imm) > 0)
			{
				is_label = true;
				int_flag = 1; // treat as valid
			}
			else
			{
				*valid = 0;
				cout<<"syntax error at line "<<line_count<<": not valid immediate value or unknown label: "<<imm<<endl;
				if(write == 1)
					log<<"syntax error at line "<<line_count<<": not valid immediate value or unknown label"<<endl;
				return;
			}
		}
		else
		{
			for(int id=1; id<imm.length(); id++)
	        {
	        	if(!(imm[id]>='0' && imm[id]<='9'))
				{
					int_flag = 0;
				}
	        }
		}

		if(!(registers.count(r1)>0 && registers.count(r2)>0 && int_flag==1))
		{
			*valid = 0;
			cout<<"syntax error at line "<<line_count<<endl;
			if(write == 1)
				log<<"syntax error at line "<<line_count<<endl;
			return;
		}

		int imm_int;
		if(is_label)
		{
			imm_int = DATALABEL[imm];
		}
		else
		{
			try
			{
				int x = stoi(imm);
			}
			catch(const std::out_of_range& oor)
			{
				*valid = 0;
				cout << "Out of Range error : immediate value out of range 0..65535 at line " << line_count<<endl;
				if(write == 1)
					log << "Out of Range error : immediate value out of range 0..65535 at line " <<line_count<< endl;
				return;
			}
			imm_int = stoi(imm);
		}
		
		string imm_bin = decToBinary(imm_int);

		if(opname == "sll" || opname == "srl" || opname == "sra")
		{
			if(negative == 1)
			{
				*valid = 0;
				cout << "Out of Range error : immediate value is negative and out of range 0..31 at line " <<line_count<< endl;
				if(write == 1)
					log << "Out of Range error : immediate value is negative and out of range 0..31 at line " << line_count<<endl;
				return;
			}
			string op = "00000000000";
			string rt, rd, h, funct;
			rt = reg_op[r2];
			rd = reg_op[r1];
			funct = type3_op2[opname];

			if(imm_bin.size()>5)
			{
				*valid = 0;
				cout << "Out of Range error : immediate value out of range 0..31 at line " << line_count<<endl;
				if(write == 1)
					log << "Out of Range error : immediate value out of range 0..31 at line " << line_count<<endl;
				return;
			}
			else if(imm_bin.size() == 0)
			{
				h = "00000";
			}
			else
			{
				string temp = "";
				for(int temp_id = 0; temp_id < (5-imm_bin.size()); temp_id++)
				{
					temp += "0";
				}

				temp += imm_bin;
				h = temp;
			}

			string result = "";
			result += op;
			result += rt;
			result += rd;
			result += h;
			result += funct;

			string res_final = "";
			for(int id=0; id<result.size(); id++)
			{
				if( id!=0 && id%4 == 0)
				{
					res_final += " ";
				}

				res_final += result[id];
			}

			printhex(pc*4, file);
			pc++;
			file<<res_final<<endl;

		}
		else
		{
			string op;
			op = type3_op1[opname];
			string rt, rs, h;
			rs = reg_op[r2];
			rt = reg_op[r1];

			if(negative == 1 && (opname == "andi" || opname == "ori" || opname == "xori"))
			{
				*valid = 0;
				cout << "Out of Range error : immediate value is negative and out of range 0..65535 at line " <<line_count<< endl;
				if(write == 1)
					log << "Out of Range error : immediate value is negative and out of range 0..65535 at line " <<line_count<< endl;
				return;
			}

			if(negative == 1)
			{
				for(int id=0; id<16; id++)
				{
					if(imm_bin[id] == '0')
					{
						*valid = 0;
						cout << "Out of Range error : negative immediate value out of range at line " <<line_count<< endl;
						if(write == 1)
							log << "Out of Range error : negative immediate value out of range at line " <<line_count<< endl;
						return;
					}
				}

				string dummy = "";
				for(int id = 16; id<32; id++)
				{
					dummy += imm_bin[id];
				}

				imm_bin = dummy;
			}



			if(imm_bin.size()>16)
			{
				*valid = 0;
				cout << "Out of Range error : immediate value out of range 0..65535 at line " <<line_count<< endl;
				if(write == 1)
					log << "Out of Range error : immediate value out of range 0..65535 at line " <<line_count<< endl;
				return;
			}
			else if(imm_bin.size() == 0)
			{
				h = "0000000000000000";
			}
			else
			{
				string temp = "";
				for(int temp_id = 0; temp_id < (16-imm_bin.size()); temp_id++)
				{
					temp += "0";
				}

				temp += imm_bin;
				h = temp;
			}

			string result = "";
			result += op;
			result += rs;
			result += rt;
			result += h;

			string res_final = "";
			for(int id=0; id<result.size(); id++)
			{
				if( id!=0 && id%4 == 0)
				{
					res_final += " ";
				}

				res_final += result[id];
			}

			printhex(pc*4, file);
			pc++;
			file<<res_final<<endl;
		}
	}
	else
	{
		*valid = 0;
		cout<<"syntax error at line "<<line_count<<endl;
		if(write == 1)
			log<<"syntax error at line "<<line_count<<endl;
		return;
	}
}

//for processing instruction type2 : op r1, r2
void type4(string opname, string operand, int operand_count, int line_count, int* valid, ofstream &file, ofstream &log, int write)
{
	if(operand_count == 2)
	{
		int i = 0;
		string r1 = "";
		string r2 = "";
		while(operand[i] != ' ')
		{
			r1 += operand[i];
			i++;
		}

		int j = i+1;
		while(operand[j] != ' ')
		{
			r2 += operand[j];
			j++;
		}

		if(!(registers.count(r1)>0 && registers.count(r2)>0))
		{
			*valid = 0;
			cout<<"syntax error at line "<<line_count<<endl;
			if(write == 1)
				log<<"syntax error at line "<<line_count<<endl;
			return;
		}

		if(opname == "move")
		{
			string operand1 = operand;
			operand1 += "$0";
			operand1 += " ";
			type2("add", operand1, operand_count+1, line_count, valid, file, log, write);
		}
		else
		{
			string op, rs, rt, h, funct;
			op = "000000";
			rs = reg_op[r1];
			rt = reg_op[r2];
			h = "0000000000";
			funct = type4_op[opname];

			string result = "";
			result += op;
			result += rs;
			result += rt;
			result += h;
			result += funct;

			string res_final = "";
			for(int id=0; id<result.size(); id++)
			{
				if( id!=0 && id%4 == 0)
				{
					res_final += " ";
				}

				res_final += result[id];
			}

			printhex(pc*4, file);
			pc++;
			file<<res_final<<endl;
		}
	}
	else
	{
		*valid = 0;
		cout<<"syntax error at line "<<line_count<<endl;
		if(write == 1)
			log<<"syntax error at line "<<line_count<<endl;
		return;
	}
}

//for processing instruction type2 : op r1, (off)r2 or op r1, label
void type5(string opname, string operand, int operand_count, int line_count, int* valid, ofstream &file, ofstream &log, int write)
{
    cout << "operand: " << operand << endl;
	if(operand_count == 2)
	{
		int i = 0;
		string r1 = "";
		string str2 = "";
		while(operand[i] != ' ')
		{
			r1 += operand[i];
			i++;
		}

		int j = i+1;
		while(operand[j] != ' ')
		{
			str2 += operand[j];
			j++;
		}

		// Check if str2 is a label (no parentheses) or register+offset format
		bool is_label_format = (str2.find('(') == string::npos);
		
		if(is_label_format)
		{
			// Handle "lw $reg, label" format
			if(!(registers.count(r1)>0))
			{
				*valid = 0;
				cout<<"syntax error at line "<<line_count<<": invalid register"<<endl;
				if(write == 1)
					log<<"syntax error at line "<<line_count<<": invalid register"<<endl;
				return;
			}
			
			if(!(datalabels.count(str2)>0))
			{
				*valid = 0;
				cout<<"syntax error at line "<<line_count<<": no data label exists with this name: "<<str2<<endl;
				if(write == 1)
					log<<"syntax error at line "<<line_count<<": no data label exists with this name"<<endl;
				return;
			}
			
			string op, rs, rt, h;
			op = type5_op[opname];
			rs = "00000";  // $zero register for base
			rt = reg_op[r1];
			
			int label_addr = DATALABEL[str2];
			string addr_bin = decToBinary(label_addr);
			
			if(addr_bin.size() > 16)
				addr_bin = addr_bin.substr(std::max(0, (int)addr_bin.size() - 16));
			
			if(addr_bin.size() == 0)
			{
				h = "0000000000000000";
			}
			else
			{
				string temp = "";
				for(int temp_id = 0; temp_id < (16-addr_bin.size()); temp_id++)
				{
					temp += "0";
				}
				temp += addr_bin;
				h = temp;
			}
			
			string result = "";
			result += op;
			result += rs;
			result += rt;
			result += h;

			string res_final = "";
			for(int id=0; id<result.size(); id++)
			{
				if( id!=0 && id%4 == 0)
				{
					res_final += " ";
				}
				res_final += result[id];
			}

			printhex(pc*4, file);
			pc++;
			file<<res_final<<endl;
			return;
		}

		// Original (offset)register format processing
		char y[str2.length()+1];
	    strcpy(y, str2.c_str());
	    string str = "";
	    int token_count = 0;

	    char *token1 = strtok(y, " (");

	    while (token1 != NULL)
		{
			str += token1;
			str += " ";
	        token_count++;
	        token1 = strtok(NULL, " ,");
	    }

	    if(token_count == 2)
	    {
	    	string r2 = "";
	    	string off = "";

	    	int l = 0;
	    	while(str[l] != ' ')
	    	{
	    		off += str[l];
	    		l++;
	    	}
            cout << "offset: " << off << endl;

	    	int m = l+1;
	    	while(str[m] != ' ')
	    	{
	    		if(str[m+1] == ' ' && str[m] == ')')
	    			break;
	    		r2 += str[m];
	    		m++;
	    	}

	    	int int_flag = 1;
			for(int id=0; id<off.length(); id++)
	        {
                // the following prevented the assembler from accepting -ve offsets
	        	if(!(off[id]>='0' && off[id]<='9') && off[id] != 45)
				{
					int_flag = 0;
				}
	        }

	        if(!(registers.count(r1)>0 && registers.count(r2)>0 && int_flag==1))
	        {
                cout << "int_flag: " << int_flag << endl;
                cout << "r1: " << r1 << endl;
                cout << "r2: " << r2 << endl;
                cout << "registers.count("<<r1<<"): " << registers.count(r1) << endl;
                cout << "registers.count("<<r2<<"): " << registers.count(r2) << endl;
		        *valid = 0;
				cout<<"syntax error at line "<<line_count<<endl;
				if(write == 1)
					log<<"syntax error at line "<<line_count<<endl;
				return;
	        }

		    try
			{
				int x = stoi(off);
			}
			catch(const std::out_of_range& oor)
			{
				*valid = 0;
				cout << "Out of Range error : offset value out of range 0..65535 at line " <<line_count<< endl;
				if(write == 1)
					log << "Out of Range error : offset value out of range 0..65535 at line " <<line_count<< endl;
				return;
			}

			int off_int = stoi(off);
			string off_bin = decToBinary(off_int);
            cout << "off_int: " << off_int << endl;
            cout << "off_bin: " << off_bin << endl;

			string op, rs, rt, h;
			op = type5_op[opname];
			rs = reg_op[r2];
			rt = reg_op[r1];

            // Since I added support for 2's complement, I need to find a way to discard upper 16 bits
            cout << "before truncation, off_bin: " << off_bin << endl;
            if (off_bin.size() > 16)
                off_bin = off_bin.substr(std::min(off_bin.size(), static_cast<size_t>(16)));
            cout << "off_bin: " << off_bin << endl;
			if(off_bin.size()>16)
			{
				*valid = 0;
				cout << "Out of Range error : offset value out of range 0..65535 at line" <<line_count<< endl;
				if(write == 1)
					log << "Out of Range error : offset value out of range 0..65535 at line " <<line_count<< endl;
				return;
			}
			else if(off_bin.size() == 0)
			{
				h = "0000000000000000";
			}
			else
			{
				string temp = "";
				for(int temp_id = 0; temp_id < (16-off_bin.size()); temp_id++)
				{
					temp += "0";
				}

				temp += off_bin;
				h = temp;
			}

			string result = "";
			result += op;
			result += rs;
			result += rt;
			result += h;

			string res_final = "";
			for(int id=0; id<result.size(); id++)
			{
				if( id!=0 && id%4 == 0)
				{
					res_final += " ";
				}

				res_final += result[id];
			}

			printhex(pc*4, file);
			pc++;
			file<<res_final<<endl;

	    }
	    else
	    {
	    	*valid = 0;
			cout<<"syntax error at line "<<line_count<<endl;
			if(write == 1)
				log<<"syntax error at line "<<line_count<<endl;
			return;
	    }
	}
	else
	{
		*valid = 0;
		cout<<"syntax error at line "<<line_count<<endl;
		if(write == 1)
			log<<"syntax error at line "<<line_count<<endl;
		return;
	}
}

//for processing instruction type2 : op r1, imm
void type6(string opname, string operand, int operand_count, int line_count, int* valid, ofstream &file, ofstream &log, int write)
{
	if(operand_count == 2)
	{
		int i = 0;
		string r1 = "";
		string imm = "";
		while(operand[i] != ' ')
		{
			r1 += operand[i];
			i++;
		}

		int j = i+1;
		while(operand[j] != ' ')
		{
			imm += operand[j];
			j++;
		}

		int negative = 0;
		int int_flag = 1;

		if(imm[0]=='-' && imm.size()==1)
		{
			*valid = 0;
			cout<<"syntax error at line "<<line_count<<": not valid immediate value"<<endl;
			if(write == 1)
				log<<"syntax error at line "<<line_count<<": not valid immediate value"<<endl;
			return;
		}

		if(imm[0] == '-')
		{
			negative = 1;
		}

		if(negative == 0 && !(imm[0] >= '0' && imm[0] <= '9'))
		{
			*valid = 0;
			cout<<"syntax error at line "<<line_count<<": not valid immediate value"<<endl;
			if(write == 1)
				log<<"syntax error at line "<<line_count<<": not valid immediate value"<<endl;
			return;
		}

		for(int id=1; id<imm.length(); id++)
        {
        	if(!(imm[id]>='0' && imm[id]<='9'))
			{
				int_flag = 0;
			}
        }

		if(!(registers.count(r1)>0 && int_flag==1))
		{
			*valid = 0;
			cout<<"syntax error at line "<<line_count<<endl;
			if(write == 1)
				log<<"syntax error at line "<<line_count<<endl;
			return;
		}

		try
		{
			int x = stoi(imm);
		}
		catch(const std::out_of_range& oor)
		{
			*valid = 0;
			cout << "Out of Range error : immediate value out of range 0..65535 at line" <<line_count<< endl;
			if(write == 1)
				log << "Out of Range error : immediate value out of range 0..65535 at line " <<line_count<< endl;
			return;
		}

		int imm_int = stoi(imm);
		string imm_bin = decToBinary(imm_int);

		if(opname == "li")
		{
			int x = imm_int >> 16;
			string imm_bin1 = decToBinary(x);

			if(negative == 1)
			{
				string dummy = "";
				for(int id=16; id<32; id++)
				{
					dummy += imm_bin1[i];
				}

				imm_bin1 = dummy;
			}

			string op, rt, h;
			op = "00111100000";
			rt = "00001";

			if(imm_bin1.size() == 0)
			{
				h = "0000000000000000";
			}
			else
			{
				string temp = "";
				for(int temp_id = 0; temp_id < (16-imm_bin1.size()); temp_id++)
				{
					temp += "0";
				}

				temp += imm_bin1;
				h = temp;
			}

			string result = "";
			result += op;
			result += rt;
			result += h;

			string res_final = "";
			for(int id=0; id<result.size(); id++)
			{
				if( id!=0 && id%4 == 0)
				{
					res_final += " ";
				}

				res_final += result[id];
			}

			printhex(pc*4, file);
			pc++;
			file<<res_final<<endl;

			x = imm_int & 65535;
			string imm_bin2 = decToBinary(x);

			string op1, rs1, rt1, h1;
			op1 = "001101";
			rs1 = "00001";
			rt1 = reg_op[r1];

			if(imm_bin2.size() == 0)
			{
				h1 = "0000000000000000";
			}
			else
			{
				string temp1 = "";
				for(int temp_id = 0; temp_id < (16-imm_bin2.size()); temp_id++)
				{
					temp1 += "0";
				}

				temp1 += imm_bin2;
				h1 = temp1;
			}

			result = "";
			result += op1;
			result += rs1;
			result += rt1;
			result += h1;

			res_final = "";
			for(int id=0; id<result.size(); id++)
			{
				if( id!=0 && id%4 == 0)
				{
					res_final += " ";
				}

				res_final += result[id];
			}

			printhex(pc*4, file);
			pc++;
			file<<res_final<<endl;

		}
		else //for instruction "lui"
		{
			if(negative == 1)
			{
				*valid = 0;
				cout << "Out of Range error : immediate value is negative and out of range 0..65535 at line " << line_count<<endl;
				if(write == 1)
					log << "Out of Range error : immediate value is negative and out of range 0..65535 at line " <<line_count<< endl;
				return;
			}

			string op, rt, h;
			op = "00111100000";
			rt = reg_op[r1];

			if(imm_bin.size()>16)
			{
				*valid = 0;
				cout << "Out of Range error : immediate value out of range 0..65535 at line " << line_count<<endl;
				if(write == 1)
					log << "Out of Range error : immediate value out of range 0..65535 at line " << line_count<<endl;
				return;
			}
			else if(imm_bin.size() == 0)
			{
				h = "0000000000000000";
			}
			else
			{
				string temp = "";
				for(int temp_id = 0; temp_id < (16-imm_bin.size()); temp_id++)
				{
					temp += "0";
				}

				temp += imm_bin;
				h = temp;
			}

			string result = "";
			result += op;
			result += rt;
			result += h;

			string res_final = "";
			for(int id=0; id<result.size(); id++)
			{
				if( id!=0 && id%4 == 0)
				{
					res_final += " ";
				}

				res_final += result[id];
			}

			printhex(pc*4, file);
			pc++;
			file<<res_final<<endl;
		}

	}
	else
	{
		*valid = 0;
		cout<<"syntax error at line "<<line_count<<endl;
		if(write == 1)
			log<<"syntax error at line "<<line_count<<endl;
		return;
	}
}

//for processing instruction type2 : op r1, label
void type7(string opname, string operand, int operand_count, int line_count, int* valid, ofstream &file, ofstream &log, int write)
{
    cout << "processing line " << line_count << endl;
	if(operand_count == 2)
	{
		int i = 0;
		string r1 = "";
		string lab = "";
		while(operand[i] != ' ')
		{
			r1 += operand[i];
			i++;
		}

		int j = i+1;
		while(operand[j] != ' ')
		{
			lab += operand[j];
			j++;
		}

		if(opname == "la")
		{
			if(registers.count(r1)>0 && datalabels.count(lab)==0)
			{
				*valid = 0;
				cout<<"syntax error at line "<<line_count
                <<": no label exists with this name: " << lab << endl;
				if(write == 1)
					log<<"syntax error at line "<<line_count<<": no label exists with this name"<<endl;
				return;
			}

			if(registers.count(r1)>0 && datalabels.count(lab)>0)
			{
				printhex(pc*4, file);
				pc++;
//                file<<"0011 1100 0000 0001 0001 0000 0000 0001"<<endl;
                file<<"0011 1100 0000 0001 0000 0000 0000 0000"<<endl;

                cout << "looking for label \"" << lab << "\" in DATALABEL" << endl;

				int x = DATALABEL[lab];
				string h;
				string op = "001101";
				string rs = "00001";
				string rt = reg_op[r1];

				string imm_bin = decToBinary(x);
				if(imm_bin.size() == 0)
				{
					h = "0000000000000000";
				}
				else
				{
					string temp = "";
					for(int temp_id = 0; temp_id < (16-imm_bin.size()); temp_id++)
					{
						temp += "0";
					}

					temp += imm_bin;
					h = temp;
				}

				string result = "";
				result += op;
				result += rs;
				result += rt;
				result += h;

				string res_final = "";
				for(int id=0; id<result.size(); id++)
				{
					if( id!=0 && id%4 == 0)
					{
						res_final += " ";
					}

					res_final += result[id];
				}

				printhex(pc*4, file);
				pc++;
				file<<res_final<<endl;

			}
			else
			{
				*valid = 0;
				cout<<"syntax error at line "<<line_count<<endl;
				if(write == 1)
					log<<"syntax error at line "<<line_count<<endl;
				return;
			}
		}
		else
		{
			if(registers.count(r1)>0 && labels.count(lab)>0)
			{
				string op = type7_op[opname];
				string rs = reg_op[r1];
				string rt = "00000";
				string h;

				if(opname == "bgez")
				{
					rt = "00001";
				}

				int x = LABEL[lab];
				x = x >> 2;

				string imm_bin = decToBinary(x);
				if(imm_bin.size() == 0)
				{
					h = "0000000000000000";
				}
				else
				{
					string temp = "";
					for(int temp_id = 0; temp_id < (16-imm_bin.size()); temp_id++)
					{
						temp += "0";
					}

					temp += imm_bin;
					h = temp;
				}

				string result = "";
				result += op;
				result += rs;
				result += rt;
				result += h;

				string res_final = "";
				for(int id=0; id<result.size(); id++)
				{
					if( id!=0 && id%4 == 0)
					{
						res_final += " ";
					}

					res_final += result[id];
				}

				printhex(pc*4, file);
				pc++;
				file<<res_final<<endl;

			}
			else
			{
				*valid = 0;
				cout<<"syntax error at line "<<line_count<<endl;
				if(write == 1)
					log<<"syntax error at line "<<line_count<<endl;
				return;
			}
		}
	}
	else
	{
		*valid = 0;
		cout<<"syntax error at line "<<line_count<<endl;
		if(write == 1)
			log<<"syntax error at line "<<line_count<<endl;
		return;
	}
}

//for processing instruction type2 : op r1
void type8(string opname, string operand, int operand_count, int line_count, int* valid, ofstream &file, ofstream &log, int write)
{
	if(operand_count == 1)
	{
		int i = 0;
		string r1 = "";
		while(operand[i] != ' ')
		{
			r1 += operand[i];
			i++;
		}

		if(!registers.count(r1)>0)
		{
			*valid = 0;
			cout<<"syntax error at line "<<line_count<<": not valid register"<<endl;
			if(write == 1)
				log<<"syntax error at line "<<line_count<< ": not valid register"<<endl;
			return;
		}

		string result = "";
		if(opname == "mfhi")
		{
			result += "0000000000000000";
			result += reg_op[r1];
			result += "00000010000";
		}
		else if(opname == "mflo")
		{
			result += "0000000000000000";
			result += reg_op[r1];
			result += "00000010010";
		}
		else if(opname == "jalr")
		{
			result += "000000";
			result += reg_op[r1];
			result += "000001111100000001001";
		}
		else if(opname == "jr")
		{
			result += "000000";
			result += reg_op[r1];
			result += "000000000000000001000";
		}

		string res_final = "";
		for(int id=0; id<result.size(); id++)
		{
			if( id!=0 && id%4 == 0)
			{
				res_final += " ";
			}

			res_final += result[id];
		}

		printhex(pc*4, file);
		pc++;
		file<<res_final<<endl;

	}
	else
	{
		*valid = 0;
		cout<<"syntax error at line "<<line_count<<": not valid register"<<endl;
		if(write == 1)
			log<<"syntax error at line "<<line_count<<": not valid register"<<endl;
		return;
	}
}

//for processing instruction type2 : op r1, r2, label
void type9(string opname, string operand, int operand_count, int line_count, int* valid, ofstream &file, ofstream &log, int write)
{
	if(operand_count == 3)
	{
		int i = 0;
		string r1 = "";
		string r2 = "";
		string lab = "";
		while(operand[i] != ' ')
		{
			r1 += operand[i];
			i++;
		}

		int j = i+1;
		while(operand[j] != ' ')
		{
			r2 += operand[j];
			j++;
		}

		int k = j+1;
		while(operand[k] != ' ')
		{
			lab += operand[k];
			k++;
		}

		if(!(registers.count(r1)>0 && registers.count(r2)>0))
		{
			*valid = 0;
			cout<<"syntax error at line "<<line_count<<endl;
			if(write == 1)
				log<<"syntax error at line "<<line_count<<endl;
			return;
		}

		if(!(labels.count(lab)>0))
		{
			*valid = 0;
			cout<<"syntax error at line "<<line_count<<": no label exists with this name"<<endl;
			if(write == 1)
				log<<"syntax error at line "<<line_count<<": no label exists with this name"<<endl;
			return;
		}

		string op;
		string rs = reg_op[r1];
		string rt = reg_op[r2];
		string h;

		if(opname == "beq")
			op = "000100";
		else if(opname == "bne")
			op = "000101";

		int x = LABEL[lab];
		x = x >> 2;

		string imm_bin = decToBinary(x);
		if(imm_bin.size() == 0)
		{
			h = "0000000000000000";
		}
		else
		{
			string temp = "";
			for(int temp_id = 0; temp_id < (16-imm_bin.size()); temp_id++)
			{
				temp += "0";
			}

			temp += imm_bin;
			h = temp;
		}

		string result = "";
		result += op;
		result += rs;
		result += rt;
		result += h;

		string res_final = "";
		for(int id=0; id<result.size(); id++)
		{
			if( id!=0 && id%4 == 0)
			{
				res_final += " ";
			}

			res_final += result[id];
		}

		printhex(pc*4, file);
		pc++;
		file<<res_final<<endl;

	}
	else
	{
		*valid = 0;
		cout<<"syntax error at line "<<line_count<<endl;
		if(write == 1)
			log<<"syntax error at line "<<line_count<<endl;
		return;
	}
}

//for processing instruction type2 : op r1, label (JAL with register and label)
void type11(string opname, string operand, int operand_count, int line_count, int* valid, ofstream &file, ofstream &log, int write)
{
	if(operand_count == 2)
	{
		int i = 0;
		string r1 = "";
		string lab = "";
		while(operand[i] != ' ')
		{
			r1 += operand[i];
			i++;
		}

		int j = i+1;
		while(operand[j] != ' ')
		{
			lab += operand[j];
			j++;
		}

		if(!(registers.count(r1)>0))
		{
			*valid = 0;
			cout<<"syntax error at line "<<line_count<<": invalid register"<<endl;
			if(write == 1)
				log<<"syntax error at line "<<line_count<<": invalid register"<<endl;
			return;
		}

		if(!(labels.count(lab)>0))
		{
			*valid = 0;
			cout<<"syntax error at line "<<line_count<<": no label exists with this name"<<endl;
			if(write == 1)
				log<<"syntax error at line "<<line_count<<": no label exists with this name"<<endl;
			return;
		}

		string op,h;

		if(opname == "jal")
			op = "000011";

		int x = LABEL[lab];
		x = x >> 2;

		string imm_bin = decToBinary(x);
		if(imm_bin.size() == 0)
		{
			h = "00000000000000000000000000";
		}
		else
		{
			string temp = "";
			for(int temp_id = 0; temp_id < (26-imm_bin.size()); temp_id++)
			{
				temp += "0";
			}

			temp += imm_bin;
			h = temp;
		}

		string result = "";
		result += op;
		result += h;

		string res_final = "";
		for(int id=0; id<result.size(); id++)
		{
			if( id!=0 && id%4 == 0)
			{
				res_final += " ";
			}

			res_final += result[id];
		}

		printhex(pc*4, file);
		pc++;
		file<<res_final<<endl;

	}
	else if(operand_count == 1)
	{
		// Fallback to standard JAL with just label
		type10(opname, operand, operand_count, line_count, valid, file, log, write);
	}
	else
	{
		*valid = 0;
		cout<<"syntax error at line "<<line_count<<endl;
		if(write == 1)
			log<<"syntax error at line "<<line_count<<endl;
		return;
	}
}

//for processing instruction type2 : op label
void type10(string opname, string operand, int operand_count, int line_count, int* valid, ofstream &file, ofstream &log, int write)
{
	if(operand_count == 1)
	{
		int i = 0;
		string lab = "";
		while(operand[i] != ' ')
		{
			lab += operand[i];
			i++;
		}

		if(!(labels.count(lab)>0))
		{
			*valid = 0;
			cout<<"syntax error at line "<<line_count<<": no label exists with this name"<<endl;
			if(write == 1)
				log<<"syntax error at line "<<line_count<<": no label exists with this name"<<endl;
			return;
		}

		string op,h;

		if(opname == "j")
			op = "000010";
		else if(opname == "jal")
			op = "000011";

		int x = LABEL[lab];
		x = x >> 2;

		string imm_bin = decToBinary(x);
		if(imm_bin.size() == 0)
		{
			h = "00000000000000000000000000";
		}
		else
		{
			string temp = "";
			for(int temp_id = 0; temp_id < (26-imm_bin.size()); temp_id++)
			{
				temp += "0";
			}

			temp += imm_bin;
			h = temp;
		}

		string result = "";
		result += op;
		result += h;

		string res_final = "";
		for(int id=0; id<result.size(); id++)
		{
			if( id!=0 && id%4 == 0)
			{
				res_final += " ";
			}

			res_final += result[id];
		}

		printhex(pc*4, file);
		pc++;
		file<<res_final<<endl;

	}
	else
	{
		*valid = 0;
		cout<<"syntax error at line "<<line_count<<endl;
		if(write == 1)
			log<<"syntax error at line "<<line_count<<endl;
		return;
	}
}

//classifies the instruction based on its syntax of operands
void instruction_process(string s, int line_count, int* valid, ofstream &file, ofstream &log, int write)
{


		int i = 0;
		string s1 = "";
		string s2 = "";
		while(s[i] != ' ')
		{
			s1 += s[i];
			i++;
		}

		int k = i+1;
		while(k < s.size()-1)
		{
			s2 += s[k];
			k++;
		}

		char y[s2.length()+1];
	    strcpy(y, s2.c_str());
	    int token_count = 0;
	    string str = "";

	    char *token1 = strtok(y, " ,");

	    while (token1 != NULL)
		{
			str += token1;
			str += " ";
	        token_count++;
	        token1 = strtok(NULL, " ,");
	    }

		int syntax_type = instr_type[s1];

		switch(syntax_type)
		{
			case 1:
			{
				type1(s1, str, token_count, line_count, valid, file, log, write);
				break;
			}
			case 2:
			{
				type2(s1, str, token_count, line_count, valid, file, log, write);
				break;
			}
			case 3:
			{
				type3(s1, str, token_count, line_count, valid, file, log, write);
				break;
			}
			case 4:
			{
				type4(s1, str, token_count, line_count, valid, file, log, write);
				break;
			}
			case 5:
			{
				type5(s1, str, token_count, line_count, valid, file, log, write);
				break;
			}
			case 6:
			{
				type6(s1, str, token_count, line_count, valid, file, log, write);
				break;
			}
			case 7:
			{
				type7(s1, str, token_count, line_count, valid, file, log, write);
				break;
			}
			case 8:
			{
				type8(s1, str, token_count, line_count, valid, file, log, write);
				break;
			}
			case 9:
			{
				type9(s1, str, token_count, line_count, valid, file, log, write);
				break;
			}
			case 10:
			{
				type10(s1, str, token_count, line_count, valid, file, log, write);
				break;
			}
			case 11:
			{
				type11(s1, str, token_count, line_count, valid, file, log, write);
				break;
			}
			default:
			{
				break;
			}
		}
}

//stores the address of labels of data section if valid
void datalabel_process(string s, int line_count, int pc_counter, int* valid, ofstream &log, int write)
{
    cout << "entered function: datalabel_process" << endl;
    cout << "current address: " << address << endl;
    cout << "Received String: " << s << endl;
	//checks that labels of data section must come under .data and not under .text
	if(!(section[0] != -1 && !(section[0] < section[1] && section[1] < pc_counter )))
	{
		*valid = 0;
		cout<<"syntax error at line "<<line_count<<" : can't put data in text segment"<<endl;
		if(write == 1)
			log<<"syntax error at line "<<line_count<<" : can't put data in text segment"<<endl;
		return;
	}


	int i = 0;
	string s1 = "";
	while(s[i] != ' ')
	{
		s1 += s[i];
		i++;
	}
    cout << "s1 = " << s1 << "|" << endl;
    cout << "i: " << i << endl;

	int idx = s1.size()-1;
    const bool is_label = (s1[idx] == ':');
	if(is_label)
	{
		string s2;
		s2 = s1;
		s2.pop_back();
        data_section_symbol_table.add_entry(s2);
        cout << "s2: " << s2 << endl;

		int j = i+1;
		string s3 = "";

		//data_type
		int data = 0;
		while(s[j] != ' ')
		{
			s3 += s[j];
			j++;
		}
        cout << "s3: " << s3 << endl;

		if(s3 == ".space")
		{
			data = 1;
		}
		else
		{
			if(s3 == ".ascii")
			{
				data = 2;
			}
			else if(s3 == ".asciiz")
			{
				data = 3;
			}
			else if(s3 == ".word")
			{
				data = 4;
			}
			else
			{
				*valid = 0;
				cout<<"syntax error at line "<<line_count<<" : wrong data type"<<endl;
				if(write == 1)
					log<<"syntax error at line "<<line_count<<" : wrong data type"<<endl;
				return;
			}
		}

		int k = j+1;
		if(k >= s.size())
		{
			*valid = 0;
			cout<<"syntax error at line "<<line_count<<": no data type specified"<<endl;
			if(write == 1)
				log<<"syntax error at line "<<line_count<<": no data type specified"<<endl;
			return;
		}
		string s4 = "";
		string s5 = "";
		//space
		while(s[k] != ' ')
		{
			s4 += s[k];
			k++;
		}

		int k1 = j+1;
		while(k1 < s.size()-1)
		{
			s5 += s[k1];
			k1++;
		}

        cout << "s4: " << s4 << endl;
        cout << "s5: " << s5 << endl;

		if(data == 1)//.space
		{
			if(k == s.size()-1)
			{
				int id = 0;
				for(id = 0; id < s4.size(); id++)
				{
					if(!(s4[id]>='0' && s4[id]<='9'))
					{
						*valid = 0;
						cout<<"syntax error at line "<<line_count<<": given space value is not an integer"<<endl;
						if(write == 1)
							log<<"syntax error at line "<<line_count<<": given space value is not an integer"<<endl;
						return;
					}
				}

				try
				{
					int x = stoi(s4);
				}
				catch(const std::out_of_range& oor)
				{
					*valid = 0;
					cout << "Out of Range error : space value out of range at line " << line_count << endl;
					if(write == 1)
						log << "Out of Range error : space value out of range at line " << line_count<<endl;
					return;
				}

				int space = stoi(s4);
                cout << "space: " << space << endl;
				datalabels.insert(s2);
                data_section_symbol_table[s2]._types.push_back(createBinding<BindingType::SPACE>(space));
                data_section_symbol_table[s2].offset = address;
				DATALABEL.insert(pair<string, int>(s2, address));
				address += space * 4;
			}
			else
			{
				*valid = 0;
				cout<<"syntax error at line "<<line_count<<endl;
				if(write == 1)
					log<<"syntax error at line "<<line_count<<endl;
				return;
			}
		}
		else if(data == 2)//.ascii
		{
			int id = s5.size()-1;
			if(s5[0] == 34 && s5[id] == 34)
			{
                data_section_symbol_table[s2]._types.push_back(createBinding<BindingType::ASCII>(s5));
                data_section_symbol_table[s2].offset = address;
				datalabels.insert(s2);
				DATALABEL.insert(pair<string, int>(s2, address));
				address += (s5.size()-2) * 4;
			}
			else
			{
				*valid = 0;
				cout<<"syntax error at line "<<line_count<<endl;
				if(write == 1)
					log<<"syntax error at line "<<line_count<<endl;
				return;
			}
		}
		else if(data == 3)//.asciiz
		{
            cout << "Processing asciiz" << endl;
            cout << "current address value is " << address << endl;
			int id = s5.size()-1;
			if(s5[0] == 34 && s5[id] == 34) // if the string ends in quotation marks
			{
                data_section_symbol_table[s2]._types.push_back(createBinding<BindingType::ASCIIZ>(s5));
                data_section_symbol_table[s2].offset = address;

				datalabels.insert(s2);
				DATALABEL.insert(pair<string, int>(s2, address));
				address += (s5.size()-2) * 4; // -2 because the string starts and ends in quotation marks
			}
			else
			{
				*valid = 0;
				cout<<"syntax error at line "<<line_count<<endl;
				if(write == 1)
					log<<"syntax error at line "<<line_count<<endl;
				return;
			}
            cout << "Done procesing asciiz. Current address is " << address << endl;
		}
		else if(data == 4)//.word
		{

			int token_count = 0;

		    char y[s5.length()+1];
		    strcpy(y, s5.c_str());

		    char *token1 = strtok(y, " ,");

		    while (token1 != NULL)
			{
                data_section_symbol_table[s2]._types.push_back(createBinding<BindingType::WORD>(atoi(token1)));
                cout << "token1: " << token1 << endl;
		        for(int id=0; id<strlen(token1); id++)
		        {
		        	if(!(token1[id]>='0' && token1[id]<='9'))
					{
						*valid = 0;
						cout<<"syntax error at line "<<line_count<<": word values are not integer"<<endl;
						if(write == 1)
							log<<"syntax error at line "<<line_count<<": word values are not integer"<<endl;
						return;
					}
		        }
		        token_count++;
		        token1 = strtok(NULL, " ,");
		    }
            data_section_symbol_table[s2].offset = address;

			int space = token_count*4;
            cout << "inserting " << s2 << " into datalabels with address " << address << endl;
			datalabels.insert(s2);
			DATALABEL.insert(pair<string, int>(s2, address));
			address += space;
		}
	}
	else
	{
		int j = i+1;
        cout << "j: " << j << endl;
		string s3 = "";

		//data_type
		int data = 0;
		while(s[j] != ' ')
		{
			s3 += s[j++];
		}

        s3 = s1;

        cout << "s3 = |" << s3 << "|" << endl;
		if(s3 == ".space")
		{
			data = 1;
		}
		else
		{
			if(s3 == ".ascii")
			{
				data = 2;
			}
			else if(s3 == ".asciiz")
			{
				data = 3;
			}
			else if(s3 == ".word")
			{
				data = 4;
			}
			else
			{
                if (label_address.find(s3) == label_address.end()) {
                    label_address[s3] = pc;
                }
			}
		}
        cout << "data: " << data << endl;

		int k = i+1;
		if(k >= s.size())
		{
			*valid = 0;
			cout<<"syntax error at line "<<line_count<<": no data type specified"<<endl;
			if(write == 1)
				log<<"syntax error at line "<<line_count<<": no data type specified"<<endl;
			return;
		}
		string s4 = "";
		string s5 = "";
		//space
		while(s[k] != ' ')
		{
			s4 += s[k];
			k++;
		}

		int k1 = i+1;
		while(k1 < s.size()-1)
		{
			s5 += s[k1];
			k1++;
		}

        cout << "s4 = |" << s4 << "|" << endl;
        cout << "s5 = |" << s5 << "|" << endl;

		if(data == 1)//.space
		{
			if(k == s.size()-1)
			{
				int id = 0;
				for(id = 0; id < s4.size(); id++)
				{
					if(!(s4[id]>='0' && s4[id]<='9'))
					{
						*valid = 0;
						cout<<"syntax error at line "<<line_count<<": given space value is not an integer"<<endl;
						if(write == 1)
							log<<"syntax error at line "<<line_count<<": given space value is not an integer"<<endl;
						return;
					}
				}

				try
				{
					int x = stoi(s4);
				}
				catch(const std::out_of_range& oor)
				{
					*valid = 0;
					cout << "Out of Range error : space value out of range at line " << line_count << endl;
					if(write == 1)
						log << "Out of Range error : space value out of range at line " << line_count<<endl;
					return;
				}

				int space = stoi(s4);
				datalabels.insert(s1);
				DATALABEL.insert(pair<string, int>(s1, address));
				address += space;
			}
			else
			{
				*valid = 0;
				cout<<"syntax error at line "<<line_count<<endl;
				if(write == 1)
					log<<"syntax error at line "<<line_count<<endl;
				return;
			}
		}
		else if(data == 2)//.ascii
		{
			int id = s5.size()-1;
			if(s5[0] == 34 && s5[id] == 34)
			{
				datalabels.insert(s1);
				DATALABEL.insert(pair<string, int>(s1, address));
				address += s5.size()-2;
			}
			else
			{
				*valid = 0;
				cout<<"syntax error at line "<<line_count<<endl;
				if(write == 1)
					log<<"syntax error at line "<<line_count<<endl;
				return;
			}
		}
		else if(data == 3)//.asciiz
		{
            cout << "processing asciiz" << endl;
			int id = s5.size()-1;
			if(s5[0] == 34 && s5[id] == 34)
			{
				datalabels.insert(s1);
				DATALABEL.insert(pair<string, int>(s1, address));
				address += s5.size()-2;
			}
			else
			{
				*valid = 0;
				cout<<"syntax error at line "<<line_count<<endl;
				if(write == 1)
					log<<"syntax error at line "<<line_count<<endl;
				return;
			}
		}
		else if(data == 4)//.word
		{

			int token_count = 0;

		    char y[s5.length()+1];
		    strcpy(y, s5.c_str());

		    char *token1 = strtok(y, " ,");
            cout << "token1: " << token1 << endl;

            bool ints, chars;
            ints = chars = false;
		    while (token1 != nullptr)
			{
                // valid input:
                //  .word 1,2,3,......
                //  .word lbl1
		        for(int id=0; id<strlen(token1); id++)
		        {
                    if (token1[id] > 47 && token1[id] < 58) ints = true;
                    else chars = true;
		        }
		        token_count++;
                {
                    bool ints_only = ints && !chars;
                    string s_beta = data_section_symbol_table._last_inserted_label;

                    if (ints_only) {
                        data_section_symbol_table[s_beta]._types.push_back(
                                createBinding<BindingType::WORD>(atoi(token1)));
                    } else {
                        data_section_symbol_table[s_beta]._types.push_back(
                                createBinding<BindingType::WORD>(-1));
                        unresolved_symbols[token1] =
                                &static_cast<WordBinding*>(data_section_symbol_table[s_beta]._types.back().get())->value;
                    }

                    data_section_symbol_table[s_beta].offset = address;
                }
                if (ints && chars && token_count > 1 || chars && token_count > 1)
                {
                    *valid = 0;
                    cout<<"syntax error at line "<<line_count
                    <<": word values cannot be multiple labels or a mixture of labels and ints"<<endl;
                    if(write == 1)
                        log<<"syntax error at line "<<line_count
                        <<": word values cannot be multiple labels or a mixture of labels and ints"<<endl;
                    return;
                }
		        token1 = strtok(NULL, " ,");
		    }
            bool alphanum = (chars && ints) || chars;
            if (alphanum) cout << "token is alphanumeric" << endl;
            else cout << "token is numeric only" << endl;
			int space = token_count*4;
            string s_alpha = "#" + s5;
            cout << "inserting \"" << s_alpha << "\" into datalabels with address " << address << endl;
			datalabels.insert(s_alpha);
			DATALABEL.insert(pair<string, int>(s_alpha, address));

			address += space;
		}
	}
    cout << "exited function datalabel_process with address: " << address << endl;
}

//sends valid lines of different types for procsessing into respective functions for their processing
void process(string s, int type, int line_count, int pc_counter, int* valid, ofstream &file, int pass, ofstream &log, int write)
{
    cout << "Entered function: process" << endl;
    cout << "working with string: " << s << endl;

	int i = 0;
	string s1 = "";
	while(s[i] != ' ')
	{
		s1 += s[i];
		i++;
	}

	if(type == 0 && pass == 0)//invalid
	{
		*valid = 0;
		cout<<"syntax error at line "<<line_count<<endl;
		if(write == 1)
			log<<"syntax error at line "<<line_count<<endl;
	}
	else if(type == 1 && pass == 0)//label
	{
		int idx = s1.size()-1;
		if(s1[idx] == ':')
		{
			string s2 = "";
			s2 = s1;
			s2.pop_back();
			if(labels.count(s2)>0 || datalabels.count(s2)>0)
			{
				*valid = 0;
				cout<<"syntax error at line "<<line_count
                <<" : label with same name declared more than once in file"<<endl;
				if(write == 1)
					log<<"syntax error at line "<<line_count
                    <<" : label with same name declared more than once in file"<<endl;
			}
			else
			{
                cout << endl;
				if(i == s.size()-1)
				{
                    int offset = pc * 4;
                    if (current_section == 3) // currently processing the data section
                        offset = address;

                    cout << endl;
					labels.insert(s2);
					//label's address is the address of the instruction following the label
					LABEL.insert(pair<string, int>(s2, offset));

                    cout << "pc: " << pc << endl;
                    cout << "pc_counter: " << pc_counter << endl;
                    cout << "address: " << address << endl;
                    cout << "section[0]: " << section[0] << endl;
                    cout << "section[1]: " << section[1] << endl;

                    // Update datalabel in virtual method table
                    if (DATALABEL.find("#"+s2) != DATALABEL.end())
                    {
                        DATALABEL["#" + s2] = LABEL.at(s2);
                    }
                    if (current_section == 3)// in data section
                    {
                        data_section_symbol_table.add_entry(s2);
                        datalabels.insert(s2);
                        DATALABEL.insert(pair<string, int>(s2, offset));
                    }
				}
				else datalabel_process(s,line_count,pc_counter,valid,log,write);
			}
		}
		else
		{
			if(labels.count(s1)>0 || datalabels.count(s1)>0)
			{
				*valid = 0;
				cout<<"syntax error at line "<<line_count
                <<" : label with same name \""<<s1<<"\" declared more than once in file"<<endl;
				if(write == 1)
					log<<"syntax error at line "<<line_count<<" : label with same name declared more than once in file"<<endl;
			}
			else
			{
				if(i+2 == s.size()-1)
				{
                    cout << "pc: " << pc << endl;
                    cout << "pc_counter: " << pc_counter << endl;
                    cout << "address: " << address << endl;
                    cout << "section[0]: " << section[0] << endl;
                    cout << "section[1]: " << section[1] << endl;

					labels.insert(s1);
					//label's address is the address of the instruction following the label
					LABEL.insert(pair<string, int>(s1, (pc)*4));

                    // Update datalabel in virtual method table
                    if (DATALABEL.find("#"+s1) != DATALABEL.end())
                        DATALABEL["#"+s1] = LABEL.at(s1);
                    if (current_section == 3)// in data section
                    {
                        data_section_symbol_table.add_entry(s1);
                        datalabels.insert(s1);
                        DATALABEL[s1] = pc*4;
                    }
				}
				else datalabel_process(s,line_count,pc_counter,valid,log,write);
			}
		}
	}
	else if(type == 2)//instruction
	{
        cout << "processing instruction: " << s << endl;
		if(pass == 0)
		{
			if(s1 == "li" || s1 == "la")
				pc++;

			pc++;
		}
		else instruction_process(s,line_count,valid,file,log,write);
	}
	else if(type == 3 && pass == 0)//data section
	{
        cout << "processing .data section" << endl;
		if(section[0] == -1)
			section[0] = pc_counter;
		else
		{
			*valid = 0;
			cout<<"syntax error at line " << line_count << " : data section declared more than once in file"<<endl;
			if(write == 1)
				log<<"syntax error at line " << line_count << " : data section declared more than once in file"<<endl;
		}
	}
	else if(type == 4 && pass == 0)//text section
	{
		if(section[1] == -1)
			section[1] = pc_counter;
		else
		{
			*valid = 0;
			cout<<"syntax error at line " << line_count << " : text section declared more than once in file"<<endl;
			if(write == 1)
				log<<"syntax error at line " << line_count << " : text section declared more than once in file"<<endl;
		}
	}
    cout << "Exited function: process, address: " << address << endl;
}

//custom function
void processFile(std::ifstream& inputFile, std::ofstream& outputFile) {
    if (!inputFile.is_open()) {
        std::cerr << "Error opening input file." << std::endl;
        return;
    }

    if (!outputFile.is_open()) {
        std::cerr << "Error opening output file." << std::endl;
        return;
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        // Remove the first 10 characters
        line = line.substr(10);

        // Remove empty spaces
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());

        // Write the modified line to the output file
        outputFile << line << std::endl;
        //inputFile = outputFile;
    }

    std::cout << "File processed successfully." << std::endl;

    // Print stuff

    cout << "\nmap of all valid labels of data section in input file with their addresses:" << endl;
    for (auto& [k, v] : DATALABEL) {
        cout << k << " | " << v << endl;
    }

    cout << "\nset of all valid labels of data section in input file:" << endl;
    for (auto& k : datalabels) {
        cout << k << endl;
    }

    cout << "\nset of all valid labels in input file:" << endl;
    for (auto& k : labels) {
        cout << k << endl;
    }

    cout << "\nmap of all valid labels in input file with their addresses:" << endl;
    for (auto& [k, v] : LABEL) {
        cout << k << " | " << v << endl;
    }
}

//custom function

int main (int argc, char *argv[])
{

	// Make sure correct number of arguments input
	// if the arguments are 3, it should be of the form ./assembler input.asm(or .s) output.txt
	// if the arguments are 4, it should be of the form ./assembler input.asm(or .s) output.txt log.txt
	if (argc < 3 || argc > 4)
	{
		printf("Incorrect number of arguments");
	}

	else
	{
		int generate = 1;//if there is no error in file generate = 1 else 0
		string line;
		int line_count = 0;//actual line number in the input file
		int valid_line_count = 0;//the modified line number for valid lines removing the lines that are empty or contain only comment

		// Open I/O files
		// Check that files opened properly
		ofstream Out;
		Out.open (argv[2]);
		if(!Out.is_open())
		{
			cout<<"Input (TXT) file could not be opened."<<endl;
			exit(1);
		}

		ofstream Log;
		int logopen = 0;
		if(argc == 4)
		{
			Log.open(argv[3], ios::app);
			if(!Log.is_open())
			{
				cout<<"Log (TXT) file could not be opened."<<endl;
				exit(1);
			}
			else
			{
				logopen = 1;
			}
		}

		ifstream In;
		In.open (argv[1]);
		if(!In.is_open())
		{
			cout<<"Input (ASM) file could not be opened."<<endl;
			exit(1);
		}
		else
		{
			//if log file is open then write in the log file
			if(logopen == 1)
			{
				Log<<" "<<endl;
				Log<<"------new run attempt for file " << argv[1] << "--------" <<endl;
			}

			//start pass0. pass0 is for processing the data and text sections, labels and datalabels
            cout << "Running first pass for processing .data and .text sections" << endl;
			while ( getline (In,line) )
		    {
		    	line_count++;
		    	int check = 1;

		    	if(line=="")//if line is empty, it is invalid for checking
		    		check = 0;
		    	else
		    	{
		    		//checking for line consisting of tabs and spaces only. if that is the case, then invalid for checking.
		    		int flag = 0;
		    		int i;
		    		for(i=0; i<line.size(); i++)
		    		{
		    			if(line[i] !=' ' && line[i]!= '\t')
		    			{
		    				flag = 1;
		    				break;
		    			}
		    		}
		    		if(flag==1)
		    		{
		    			if(line[i]=='/' || line[i]=='#')
		    			{
		    				check = 0;
		    			}
		    		}
		    		else
		    		{
		    			check = 0;
		    		}

		    		if(check == 1)//if valid for checking, then process the line
		    		{
		    			valid_line_count++;
		    			string str = "";
		    			char x[line.length()+1];
		    			strcpy(x, line.c_str());

		    			// remove the spaces and tabs. tokenizing the given line
		    			char *token = strtok(x, " \t");

		    			while (token != NULL)
					    {
					        str += token;
					        str += " ";
					        token = strtok(NULL, " \t");
					    }

					    //removing the comment part from the line
					    string str_final = "";
					    for(int i=0; i<str.size(); i++)
					    {
					    	if(str[i]=='/' || str[i]=='#')
					    		break;
					    	else
					    	{
					    		str_final += str[i];
					    	}
					    }

					    int last_id = str_final.size()-1;
					    if(str_final[last_id] != ' ')
					    	str_final += ' ';

					    //determine the type of line. if it is zero, the line is invalid
                        cout << endl;
					    int ans = string_type(str_final, &current_section);
                        cout << "ans = " << ans << endl;
					    //send it for processing
					    process(str_final, ans, line_count, valid_line_count, &generate, Out, 0, Log, logopen);
		    		}
		    	}
		    }
		}

		//after pass0 is over close the input file
		In.close();

		//pass1 is for processing the instructions
		// start pass1. similar to pass0 except in the process fumction we send the pass value = 1
        cout << "\nRunning second pass for processing instructions" << endl;
		pc = 0;
		line_count = 0;
		valid_line_count = 0;
		In.open (argv[1]);
		if(!In.is_open())
		{
			exit(1);
		}
		else
		{
			while ( getline (In,line) )
		    {
		    	line_count++;
		    	int check = 1;

		    	if(line=="")
		    		check = 0;
		    	else
		    	{
		    		int flag = 0;
		    		int i;
		    		for(i=0; i<line.size(); i++)
		    		{
		    			if(line[i] !=' ' && line[i]!= '\t')
		    			{
		    				flag = 1;
		    				break;
		    			}
		    		}
		    		if(flag==1)
		    		{
		    			if(line[i]=='/' || line[i]=='#')
		    			{
		    				check = 0;
		    			}
		    		}
		    		else
		    		{
		    			check = 0;
		    		}

		    		if(check == 1)
		    		{
		    			valid_line_count++;
		    			string str = "";
		    			char x[line.length()+1];
		    			strcpy(x, line.c_str());

		    			char *token = strtok(x, " \t");

		    			while (token != NULL)
					    {
					        str += token;
					        str += " ";
					        token = strtok(NULL, " \t");
					    }

					    string str_final = "";
					    for(int i=0; i<str.size(); i++)
					    {
					    	if(str[i]=='/' || str[i]=='#')
					    		break;
					    	else
					    	{
					    		str_final += str[i];
					    	}
					    }

					    int last_id = str_final.size()-1;
					    if(str_final[last_id] != ' ')
					    	str_final += ' ';

                        cout << endl;
					    int ans = string_type(str_final, &current_section);
                        cout << "ans = " << ans << endl;
					    process(str_final, ans, line_count, valid_line_count, &generate, Out, 1, Log, logopen);
		    		}
		    	}
		    }
		}

// Generate code for data section
        // fill in the missing values for the data section
        for(auto& [k, v] : LABEL) {
            cout << "k: " << k << ", v: " << v << endl;
            SymbolTableEntry temp_entry{k, {{}, -1}};
            auto& entries = data_section_symbol_table.entries;
            auto it = entries.find(temp_entry);
            if (it != entries.end()) {
                cout << "entry found in fata section symbol table." << endl;
                auto& _binding_information = const_cast<BindingInformation&>(it->_binding_information);
                if (_binding_information.offset == -1)
                    _binding_information.offset = v;
            } else {
                auto _it = unresolved_symbols.find(k);
                if (_it != unresolved_symbols.end()) {
                    *static_cast<int*>(_it->second) = v;
                }
            }
        }
        data_section_symbol_table.print();

        cout << "Starting data segment code gen..." << endl;
        DataSegmentCodegenVisitor data_codegen_visitor;
        for (auto& e : data_section_symbol_table._insertion_order) {
            const auto& _binding_information = data_section_symbol_table[e];
            for (const auto& t: _binding_information._types) {
                t->accept(&data_codegen_visitor);
            }
        }

        { // save the machine code for data section
            string output_no_address_filename(argv[2]);
            std::ofstream outputFile(output_no_address_filename+".no_address.data.bin");
            outputFile << data_codegen_visitor.code << std::flush;
        }


		//if there is error empty the output file
		if(generate == 0)
		{
			Out.close();
			Out.open(argv[2], ios::trunc);
		}
		else//no error
		{
			if(logopen == 1)
				Log<<"no error"<<endl;
		}

		// Close files
		In.close();
        Out.close();

//custom function
        string output_no_address_filename(argv[2]);
        std::ifstream inputFile(argv[2]);
        std::ofstream outputFile(output_no_address_filename+".no_address.text.bin");


        // Call the function with the file streams
        processFile(inputFile, outputFile);

        // Close the files
        inputFile.close();
        outputFile.close();

//custom function

		if(logopen == 1)
		{
			Log.close();
		}

		return 0;
	}
}
