/*
* @Author: 王楚涵
* @Date:   2018-10-24
* @Last Modified by:   王楚涵
* @Last Modified time: 2019-01-06
*/

#include <bits/stdc++.h>
#include "elf_file.h"

using namespace std;
#define pb push_back
#define _  << "  " <<


ELF_FILE elf;
int CurrAddr;//text偏移


vector<InstCode> program;//text segment

InstCode code;
map<string,int> opcode;
map<string,int> global_index;// record the global variable with the offset in symtab
map<string,int> section_index;//record the section name with the offset in shstrtab
map<string,int> shdr_index;//record the section name with the index in shdrtab

vector<string> inst;
int align;//对齐标志
int register_used;


/**
 * @Author    Wang_Chuhan    Wang_Zitai
 * @DateTime  2019-01-06
 * @copyright [Copyright (c)2019]
 * @param     s              [description]
 */
void split(string s)//指令参数分割
{
	int state = 0;
	int j = 0;
	inst.pb("");
	for(int i = 0; i<s.length(); i++){
		//cout << s[i] << endl;
		if(s[i]==' ' || s[i]=='\t' || s[i]=='\n' || s[i]==',' || s[i]=='(' || s[i]==')' || s[i]=='\r')
		{
			if(state==1){
				j++;
				inst.pb("");
			}
			state = 0;
			continue;
		}
		else{
            state = 1;
			if(s[i]=='"'){
				i++;
				while(i<s.length() && s[i]!='"'){
					inst[j] += s[i];
					i++;
				}
			}
			else{
                inst[j] += s[i];
			}
		}
	}
}

void StrToInt(int &int_temp,const string &string_temp)
{
    stringstream stream(string_temp);
    stream>>int_temp;
}

int AddGlobalSym(string s)
{
    int id;
    if(global_index.find(s)!=global_index.end())
    {
        id = global_index[s];
    }
    else{
        int st_name = elf.strtab.size();
        //add the string to strtab
        //cout << "*******************" _ s << endl;
        string name = s;
        for(int i = 0; i<name.size(); i++)
            elf.strtab.pb(name[i]);
        elf.strtab.pb('\0');

        int st_value = 0;
        int st_size = 0;
        unsigned char st_info = ELF32_ST_INFO( STB_GLOBAL, STT_NOTYPE);// bind << 4 + (type & 0xf)
        unsigned char st_other = STV_DEFAULT;// 0
        Elf32_Section st_shndx = 0;
        id = elf.symtab.size();
        elf.symtab.pb(Elf32_Sym{ st_name, st_value, st_size, st_info, st_other, st_shndx} );
        global_index[s] = id;
    }
    return id;
}

/**
 * @Author      Wang_Chuhan    Wang_Zitai
 * @DateTime    2019-01-06
 * @copyright   [Copyright (c)2019]
 * @description [读入指令 分割 将对应的操作码操作数写入对应的指令的数据结构]
 * @return      [读入的指令本身 用于判断循环结束]
 */
string Handle_One_Instruction()
{
	string s;
	inst.clear();

	if(getline(cin,s)){
		//cout << s << endl;
		split(s);
		printf("%-4x",CurrAddr);
		if(s.length()==0)
			return "blank";
		for(int i = 0; i<inst.size(); i++){
			cout << inst[i] << " ";
		}
		cout <<endl;
		if(inst[0][1]=='#')
			return "note";
		else if(inst[0]==".align"){
			StrToInt(align, inst[1]);
		}

		else if(inst[0]==".rdata"){
			//创建rodata符号表
			int st_name = 0;
			int st_value = 0;
			int st_size = 0;
			unsigned char st_info = ELF32_ST_INFO( STB_LOCAL, STT_SECTION);// bind << 4 + (type & 0xf)
			unsigned char st_other = STV_DEFAULT;// 0
			///////////////
			Elf32_Section st_shndx = 3;
			//we can only get the num after pushing all the section into the shdrtab
			//so we prefer to set the variable fixed
			//////////////
			global_index["rodata"] = elf.symtab.size();
			elf.symtab.pb(Elf32_Sym{ st_name, st_value, st_size, st_info, st_other, st_shndx} );

		}
		else if(inst[0]==".text"){
			//创建text符号表
			int st_name = 0;
			int st_value = 0;
			int st_size = 0;
			unsigned char st_info = ELF32_ST_INFO( STB_LOCAL, STT_SECTION);// bind << 4 + (type & 0xf)
			unsigned char st_other = STV_DEFAULT;// 0
			///////////////
			Elf32_Section st_shndx = 1; // waiting to update
			//////////////
			elf.symtab.pb(Elf32_Sym{ st_name, st_value, st_size, st_info, st_other, st_shndx} );
		}
		else if(inst[0]==".end" && inst[1]=="main"){
			//添加main符号表 记录整个main函数大小
			int st_name = elf.strtab.size();
			//add the string to strtab
			string name = inst[1];
			for(int i = 0; i<name.size(); i++)
				elf.strtab.pb(name[i]);
			elf.strtab.pb('\0');

			int st_value = 0;
			int st_size = CurrAddr;
			unsigned char st_info = ELF32_ST_INFO( STB_GLOBAL, STT_FUNC);// bind << 4 + (type & 0xf)
			unsigned char st_other = STV_DEFAULT;// 0
			Elf32_Section st_shndx = 1; // 1
			elf.symtab.pb(Elf32_Sym{ st_name, st_value, st_size, st_info, st_other, st_shndx} );
		}
		else if(inst[0]==".file"){
			//file添加到符号表
			int st_name = elf.strtab.size();
			//add the string to strtab
			string name = inst[2];
			for(int i = 0; i<name.size(); i++)
				elf.strtab.pb(name[i]);
			elf.strtab.pb('\0');

			int st_value = 0;
			int st_size = 0;
			unsigned char st_info = ELF32_ST_INFO( STB_LOCAL, STT_FILE);// bind << 4 + (type & 0xf)
			unsigned char st_other = STV_DEFAULT;// 0
			Elf32_Section st_shndx = SHN_ABS; // 0xfff1
			elf.symtab.pb(Elf32_Sym{ st_name, st_value, st_size, st_info, st_other, st_shndx} );
		}
		else if(inst[0]==".cprestore"){
			//汇编器指令 执行一些固定操作
			code.op = opcode["sw"];
			code.r1 = 29;
			StrToInt(code.tail, inst[1]);
			//code.tail = 16;
			code.r2 = 28;
			code.print();
			program.pb(code);
			CurrAddr += 4;
		}
		else if(inst[0][0]=='$'){
			//处理字符串常量
			int len = inst[0].length();
			string name = inst[0].substr(0,len-1);
			elf.rodata_offset[name] = elf.rodata.size();
			//cout << name << endl;
			getline(cin,s);
			inst.clear();//clear the inst part vector
			split(s);
			//对空格回车的翻译
            string out;
            for(int i = 0; i < inst[1].size(); i++){
                if(inst[1][i] == '\\'){
                    string temp = inst[1].substr(i+1, 3);
                    if(temp == "000")
                        out += '\0';
                    else if (temp == "012")
                        out += '\n';
                    i += 3;
                }
                else
                    out += inst[1][i];
            }
            //写入rodata表
			for(int i = 0; i<out.size(); i++)
                elf.rodata.pb(out[i]);
            if(out.size()%align != 0)
                for(int i = 0; i<align-(out.size()%align); i++ )
                    elf.rodata.pb('\0');
			//elf.rodata.pb('a');
		}
		else if(inst[0]=="addiu"){

			code.op = opcode[inst[0]];
			if(inst[1][0]=='$'){
				StrToInt(code.r2,inst[1].substr(1));
				register_used |= (1<<code.r2);
			}
			else
				StrToInt(code.r2,inst[1]);
			if(inst[2][0]=='$'){
				StrToInt(code.r1,inst[2].substr(1));
				register_used |= (1<<code.r1);
			}
			else
				StrToInt(code.r1,inst[2]);
			//处理第三个参数是%lo %hi的情况
			if(inst[3]=="%lo"){
				//如果是low的 立即数填写对应标识符在rowtext的偏移 填写reltext段
				if(inst[4][0]=='$'){
					code.tail = elf.rodata_offset[inst[4]];

					int id = AddGlobalSym("rodata");
					elf.reltext.pb( Elf32_Rel{ Elf32_Addr(CurrAddr) , ELF32_R_INFO( (Elf32_Word)id , R_MIPS_LO16) } );
				}
				else{
					code.tail = 0;
					int id = AddGlobalSym(inst[4]);
					elf.reltext.pb( Elf32_Rel{ Elf32_Addr(CurrAddr) , ELF32_R_INFO( (Elf32_Word)id , R_MIPS_LO16) } );
				}
				//elf.reltext.pb( Elf32_Rel{ Elf32_Addr(CurrAddr) , ELF32_R_INFO( (Elf32_Word)id , R_MIPS_LO16) } );
			}
			else
				StrToInt(code.tail,inst[3]);
			program.pb(code);
			code.print();

			CurrAddr += 4;
		}
		else if(inst[0]=="sw" || inst[0]=="lw"){
			// index in inst 	inst[index]
            // 		0    			LW
            // 		1    			$25
            // 		2    			%call16
            // 		3    			printf
            // 		4    			$28
            // need write reltext symtable tail = 0
			code.op = opcode[inst[0]];
			if(inst[1][0]=='$'){
				StrToInt(code.r2,inst[1].substr(1));
				register_used |= (1<<code.r2);
			}
			else
				StrToInt(code.r2,inst[1]);
            if(inst[2][0]=='%'){
            	code.tail = 0;
            	StrToInt(code.r1,inst[4].substr(1));
            	int id = AddGlobalSym(inst[3]);
				elf.reltext.pb( Elf32_Rel{ Elf32_Addr(CurrAddr) , ELF32_R_INFO( (Elf32_Word)id , R_MIPS_CALL16) } );
				//printf("CALL**  offset %x  info %x\n",Elf32_Addr(CurrAddr), ELF32_R_INFO( (Elf32_Word)id , R_MIPS_CALL16));
            }
            else{
                StrToInt(code.tail,inst[2]);
                if(inst[3][0]=='$'){
					StrToInt(code.r1,inst[3].substr(1));
					register_used |= (1<<code.r1);
                }
				else
					StrToInt(code.r1,inst[3]);
            }

			code.print();
			program.pb(code);
			CurrAddr += 4;
		}
		else if(inst[0]=="move"){

			code.op = opcode[inst[0]];
			StrToInt(code.r1,inst[2].substr(1));
			register_used |= (1<<code.r1);
			code.r2 = 0;
			int num;
			StrToInt(num,inst[1].substr(1));
			register_used |= (1<<num);
			code.tail = (num<<11) + 33;
			code.print();
			program.pb(code);

			CurrAddr += 4;
		}
		else if(inst[0]=="jalr"){

			code.op = opcode[inst[0]];
			StrToInt(code.r1,inst[1].substr(1));
			register_used |= (1<<code.r1);
			code.r2 = 0;
			code.tail = (31<<11) + 9;
			code.print();
			program.pb(code);

			CurrAddr += 4;
		}
		else if(inst[0]=="nop"){
			code.op = opcode[inst[0]];
			code.r1 = 0;
			code.r2 = 0;
			code.tail = 0;
			code.print();
			program.pb(code);

			CurrAddr += 4;
		}
		else if(inst[0]=="j"){

			code.op = opcode[inst[0]];
			StrToInt(code.r1,inst[1].substr(1));
			register_used |= (1<<code.r1);
			code.r2 = 0;
			code.tail = 8;
			code.print();
			program.pb(code);

			CurrAddr += 4;
		}
		else if(inst[0]=="li"){

			code.op = opcode[inst[0]];
			if(inst[1][0]=='$'){
				StrToInt(code.r2,inst[1].substr(1));
				register_used |= (1<<code.r2);
			}
			else
				StrToInt(code.r2,inst[1]);
			code.r1 = 0;
			StrToInt(code.tail,inst[2]);
			program.pb(code);
			code.print();

			CurrAddr += 4;
		}
		else if(inst[0]=="sll"){

			code.op = opcode[inst[0]];
			code.r1 = 0;
			StrToInt(code.r2,inst[2].substr(1));
			register_used |= (1<<code.r2);
			int num;
			StrToInt(num,inst[1].substr(1));
			register_used |= (1<<num);
			int num1;
			StrToInt(num1,inst[3]);
			code.tail = (num<<11) + (num1<<6);
			code.print();
			program.pb(code);

			CurrAddr += 4;
		}
		else if(inst[0]=="addu"){
			code.op = opcode[inst[0]];
			StrToInt(code.r1,inst[2].substr(1));
			register_used |= (1<<code.r1);
			StrToInt(code.r2,inst[3].substr(1));
			register_used |= (1<<code.r2);
			int num;
			StrToInt(num,inst[1].substr(1));
			register_used |= (1<<num);
			code.tail = (num<<11) +33;
			code.print();
			program.pb(code);
			CurrAddr += 4;
		}
		else if(inst[0]=="lui"){
			code.op = opcode[inst[0]];
			code.r1 = 0;
			StrToInt(code.r2,inst[1].substr(1));
			register_used |= (1<<code.r2);
			//处理%hi情况
			if(inst[2]=="%hi"){
                //如果是high  立即数填写0 填写reltext段
                if(inst[3][0]=='$'){
                	code.tail = 0;
                	int id = AddGlobalSym("rodata");
                	elf.reltext.pb( Elf32_Rel{ Elf32_Addr(CurrAddr) , ELF32_R_INFO( (Elf32_Word)id , R_MIPS_HI16) } );
                }
                else{
                	int id = AddGlobalSym(inst[3]);
					code.tail = 0;
					elf.reltext.pb( Elf32_Rel{ Elf32_Addr(CurrAddr) , ELF32_R_INFO( (Elf32_Word)id , R_MIPS_HI16) } );
					//printf("HI**  offset %x  info %x\n",Elf32_Addr(CurrAddr), ELF32_R_INFO( (Elf32_Word)id , R_MIPS_HI16));
				}
				//cout << "**" _ Elf32_Addr(CurrAddr) _ ELF32_R_INFO( (Elf32_Word)id , R_MIPS_HI16) << endl;
			}
			else
				code.tail = 888888; //这里先不处理 不知道处理什么？？？
				//在我们的测试代码中并没有遇到else的情况 所以可以任意赋值
			code.print();
			program.pb(code);
			CurrAddr += 4;
		}
	}
	return s;
}

/**
 * @Author      Wang_Chuhan    Wang_Zitai
 * @DateTime    2019-01-06
 * @copyright   [Copyright (c)2019]
 * @description [将段表名写入到section header string table中]
 * @param       s              [段表名]
 */
void Addshstrtab(string s)
{
	//维护段表名字符串起始位置
	section_index[s] = elf.shstrtab.size();
	for(int i = 0; i<s.length(); i++)
		elf.shstrtab.pb(s[i]);
	elf.shstrtab.pb('\0');
	//写入\0结束
}

int getsize(int section_size, int section_align)
{
    return (section_size + section_align - 1) / section_align * section_align;
}

void writeELF()
{
	//cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << elf.shdrtab.size() << endl;
	long long offset;
	FILE *fout = fopen("all.o","wb");
	/*
		windows下参数为wb 写二进制文件 使用参数w 会自动在0A前面添加0D
		For Windows parameter should be 'wb'
		For Linux parameter 'w' or 'wb' is okay
	*/
	// write elf file head
	fwrite(&elf.ehdr,elf.ehdr.e_ehsize,1,fout);
	//write the null char for align
	char pad[100] = {0};
	offset = ftell(fout);
	int num = elf.shdrtab[1].sh_offset - offset;
	fwrite(pad,1,num,fout);
	// write .text segment
	for(int i = 0; i<program.size(); i++){
		int instruction = program[i].cal();
		fwrite(&instruction,4,1,fout);
	}

	//write the null char for align
	offset = ftell(fout);
	num = elf.shdrtab[2].sh_offset - offset;
	fwrite(pad,1,num,fout);


	//write .reginfo segment
    fwrite(&elf.reginfo, sizeof(elf.reginfo), 1, fout);

	//write the null char for align

    //printf("^^^^^^^^^^^^^^  %x\n", elf.shdrtab[3].sh_offset);
	//cout << "&&&&&&&&&&&&" _ elf.shdrtab[3].sh_offset << endl;
	offset = ftell(fout);
	num = elf.shdrtab[3].sh_offset - offset;
	fwrite(pad,1,num,fout);

	//write .rodata segment
	for(int i = 0; i < elf.rodata.size(); i++) {
        fwrite(&elf.rodata[i], 1, 1, fout);
	}

    char comment[50] = "GCC: (GNU) 4.3.6 20101004 (prerelease)\0";
    char gnu_attributes[16] = {
      0x41, 0x0f, 0x00, 0x00, 0x00, 0x67, 0x6e, 0x75,
      0x00, 0x01, 0x07, 0x00, 0x00, 0x00, 0x04, 0x01
    };
	//write the null char for align
	offset = ftell(fout);
	num = elf.shdrtab[4].sh_offset - offset;
	//printf("^^^^^^^^^^^^^^  %x %x\n", elf.shdrtab[4].sh_offset ,offset);
	//cout << num << endl;
	fwrite(pad,1,num,fout);
	//write .comment segment
	fwrite(pad,1,1,fout);
	//cout << elf.comment << endl;
	fwrite(&comment,1,39,fout);
	//write the null char for align
	offset = ftell(fout);
	num = elf.shdrtab[5].sh_offset - offset;
	fwrite(pad,1,num,fout);
	//write .gnu.attributes segment
	fwrite(&gnu_attributes, 1, 16, fout);

	//write the null char for align
	offset = ftell(fout);
	num = elf.shdrtab[6].sh_offset - offset;
	fwrite(pad,1,num,fout);
	//write .shstrtab segment
    for(int i = 0; i < elf.shstrtab.size(); i++) {
        fwrite(&elf.shstrtab[i], 1, 1, fout);
	}
	//write the null char for align
	offset = ftell(fout);
	num = elf.shdrtab[7].sh_offset - offset;
	fwrite(pad,1,num,fout);
	//write .symtab segment
    for(int i = 0; i < elf.symtab.size(); i++) {
        fwrite(&elf.symtab[i], sizeof(Elf32_Sym), 1, fout);
    }
	//write the null char for align
	offset = ftell(fout);
	num = elf.shdrtab[8].sh_offset - offset;
	fwrite(pad,1,num,fout);
	//write .strtab segment
    for(int i = 0; i < elf.strtab.size(); i++) {
        fwrite(&elf.strtab[i], 1, 1, fout);
	}
	//write the null char for align
	offset = ftell(fout);
	num = elf.shdrtab[9].sh_offset - offset;
	fwrite(pad,1,num,fout);
	//write .rel.text segment
    for(int i = 0; i < elf.reltext.size(); i++) {
        fwrite(&elf.reltext[i], sizeof(Elf32_Rel), 1, fout);
    }
	//write the null char for align

	offset = ftell(fout);
	num = elf.ehdr.e_shoff - offset;

	fwrite(pad,1,num,fout);
	//printf("00 %x\n",ftell(fout));
	//write header table segment
	for (int i = 0; i < elf.shdrtab.size(); i++) {
        fwrite(&elf.shdrtab[i], sizeof(Elf32_Shdr), 1, fout);
	}

	fclose(fout);
}
int main()
{
	//读入汇编代码文件
	freopen("all1.s","r",stdin);
	//freopen("output.txt","w",stdout);

	//规定汇编指令的操作码
	opcode["sw"] = 43;
	opcode["lw"] = 35;
	opcode["move"] = 0;
	opcode["addiu"] = 9;
	opcode["jalr"] = 0;
	opcode["nop"] = 0;
	opcode["j"] = 0;
	opcode["li"] = 9;
	opcode["sll"] = 0;
	opcode["lui"] = 15;

	elf.symtab.pb( Elf32_Sym{0,0,0,0,0} );//symtab first push a empty item
	elf.strtab.pb('\0');//strtab first push a empty char

	// read each instruction and do something else
	while(Handle_One_Instruction()!="");


	//write shstrtab
	elf.shstrtab.pb('\0');
	Addshstrtab(".text");
	Addshstrtab(".rel.text");
	Addshstrtab(".reginfo");
	Addshstrtab(".rodata");
	Addshstrtab(".comment");
	Addshstrtab(".gnu.attributes");
	Addshstrtab(".shstrtab");
	Addshstrtab(".symtab");
	Addshstrtab(".strtab");

	elf.shdrtab.pb(Elf32_Shdr{ 0,0,0,0,0,0,0,0,0,0});

	shdr_index[".text"] = elf.shdrtab.size();
	elf.shdrtab.pb(Elf32_Shdr{ section_index[".text"], SHT_PROGBITS, SHF_ALLOC|SHF_EXECINSTR,
                                0, 0x40, (CurrAddr+16)/16*16, 0, 0, 16, 0
                             });
    int section_align = 4;
    int offset = 64 + (CurrAddr+16)/16*16;
    int section_size = 24;
    offset = getsize(offset,section_align);
    shdr_index[".reginfo"] = elf.shdrtab.size();
	elf.shdrtab.pb(Elf32_Shdr{ section_index[".reginfo"], SHT_MIPS_REGINFO, SHF_ALLOC,
                                0, offset, section_size, 0, 0, section_align , 24
                             });
    section_align = 16;
    offset += section_size;
    offset = getsize(offset,section_align);
    section_size = getsize(elf.rodata.size() , section_align);
    shdr_index[".rodata"] = elf.shdrtab.size();
    elf.shdrtab.pb(Elf32_Shdr{ section_index[".rodata"], SHT_PROGBITS, SHF_ALLOC,
                                0, offset, section_size, 0, 0, section_align , 0
                             });

    section_align = 1;
    offset += section_size;
    offset = getsize(offset,section_align);
    section_size = 40; // comment segment size we do not know
    shdr_index[".comment"] = elf.shdrtab.size();
    elf.shdrtab.pb(Elf32_Shdr{ section_index[".comment"], SHT_PROGBITS, 0,
                                0, offset, section_size, 0, 0, section_align , 0
                             });

    section_align = 1;
    offset += section_size;
    offset = getsize(offset,section_align);
    section_size = 0x10; // comment segment size we do not know
    int gnu_tmp = 0xffffff5;
    shdr_index[".gnu.attributes"] = elf.shdrtab.size();
    elf.shdrtab.pb(Elf32_Shdr{ section_index[".gnu.attributes"], SHT_LOOS + gnu_tmp, 0,
                                0, offset, section_size, 0, 0, section_align , 0
                             });

    section_align = 1;
    offset += section_size;
    offset = getsize(offset,section_align);
    section_size = elf.shstrtab.size();
    shdr_index[".shstrtab"] = elf.shdrtab.size();
    elf.shdrtab.pb(Elf32_Shdr{ section_index[".shstrtab"], SHT_STRTAB, 0,
                                0, offset, section_size, 0, 0, section_align , 0
                             });


    section_align = 4;
    offset += section_size;
    offset = getsize(offset,section_align);
    int ES = 0x10;
    section_size = getsize( ES*elf.symtab.size() ,section_align);
    int LK = elf.shdrtab.size() + 1;
    int INFO = elf.symtab.size() - global_index.size();
    shdr_index[".symtab"] = elf.shdrtab.size();
    elf.shdrtab.pb(Elf32_Shdr{ section_index[".symtab"], SHT_SYMTAB, 0,
                                0, offset, section_size, LK, INFO, section_align , ES
                             });

    section_align = 1;
    offset += section_size;
    offset = getsize(offset,section_align);
    section_size = getsize( elf.strtab.size() ,section_align);
    shdr_index[".strtab"] = elf.shdrtab.size();
    elf.shdrtab.pb(Elf32_Shdr{ section_index[".strtab"], SHT_STRTAB, 0,
                                0, offset, section_size, 0, 0, section_align , 0
                             });

    section_align = 4;
    offset += section_size;
    offset = getsize(offset,section_align);
    ES = 8;
    section_size = getsize( ES * elf.reltext.size() ,section_align);
    LK = elf.shdrtab.size() - 2;
    shdr_index[".rel.text"] = elf.shdrtab.size();
    elf.shdrtab.pb(Elf32_Shdr{ section_index[".rel.text"], SHT_REL, 0,
                                0, offset, section_size, LK, 1, section_align , ES
                             });

    //cout << register_used << endl;
    elf.reginfo.ri_gprmask = (Elf32_Word)register_used;
    elf.reginfo.ri_gprmask = elf.reginfo.ri_gprmask/2*2;
    //cout << "rodata" _ elf.rodata.size() << endl;
    //cout << elf.reltext.size() << endl;

    section_align = 16;
    offset += section_size;
    offset = getsize(offset,section_align);

    int*p_id = (int*)elf.ehdr.e_ident;
    *p_id = 0x464c457f;	p_id++;
    *p_id = 0x00010101;	p_id++;
    *p_id = 0;			p_id++;
    *p_id = 0;
    elf.ehdr.e_type = ET_REL;
    elf.ehdr.e_machine = EM_MIPS;
    elf.ehdr.e_version = EV_CURRENT;
    elf.ehdr.e_entry = 0;
    elf.ehdr.e_phoff = 0;
    elf.ehdr.e_shoff = offset;
    elf.ehdr.e_flags = 0x50a21007;
    elf.ehdr.e_ehsize = 52;
    elf.ehdr.e_phentsize = 0;
    elf.ehdr.e_phnum = 0;
    elf.ehdr.e_shentsize = 40;
    elf.ehdr.e_shnum = elf.shdrtab.size();
    elf.ehdr.e_shstrndx = shdr_index[".shstrtab"];

    writeELF();

	elf.print();
	cout << "-------------section index -------------" << endl;
    map<string,int> :: iterator it;
    for(it = section_index.begin(); it!=section_index.end(); it++){
        cout << (*it).first _ (*it).second << endl;
    }
	return 0;
}
