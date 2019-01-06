/*
	本文件封装了标准ELF文件格式。
*/

#ifndef _ELF_FILE_H
#define	_ELF_FILE_H
#define _ << " " <<
//elf文件所有数据结构和宏定义

#include "elf.h"
#include <map>
//STL模板库
#include <ext/hash_map>
#include <vector>
#include <iostream>
#include <cstdio>

using namespace std;
using namespace __gnu_cxx;


struct lb_record//符号声明记录
{
  static int curAddr;//一个段内符号的偏移累加量
  string segName;//隶属于的段名，三种：.text .data .bss
  string lbName;//符号名
  bool isEqu;//是否是L equ 1
  bool externed;//是否是外部符号，内容是1的时候表示为外部的，此时curAddr不累加
  int addr;//符号段偏移
  int times;//定义重复次数
  int len;//符号类型长度：db-1 dw-2 dd-4
  int *cont;//符号内容数组
  int cont_len;//符号内容长度
  lb_record(string n,bool ex);//L:或者创建外部符号(ex=true:L dd @e_esp)
  lb_record(string n,int a);//L equ 1
  lb_record(string n,int t,int l,int c[],int c_l);//L times 5 dw 1,"abc",L2 或者 L dd 23
  void write();//输出符号内容
  ~lb_record();
};


// 需要自己写hash函数
struct string_hash
{
  size_t operator()(const string& str) const
  {
    return __stl_hash_string(str.c_str());
  }
};

//重定位信息
struct RelInfo
{
	string tarSeg;//重定位目标段
	int offset;//重定位位置的偏移
	string lbName;//重定位符号的名称
	int type;//重定位类型0-R_386_32；1-R_386_PC32
	RelInfo(string seg,int addr,string lb,int t);
};


//.rel .text segment
struct RelText
{
    int offset;     //offset in text segment
    int info;       //calculate by ELF32_R_INFO(sym, type), which is defined in elf.h, in which sym is the index of the symbol, type is R_MIPS_REL32(also in elf.h).
    int type;       //define in elf.h, R_MIPS_HI16, R_MIPS_LO16, R_MIPS_CALL16
    int symValue;   //in our program, always 0
    string symName; //__gnu_local_gp, .rodata, puts, scanf, printf
};

struct InstCode{
	int op;//操作码 26-31
	int r1;//寄存器1 21-25
	int r2;//16-20
	int tail;//0-15
	void print(){
        //printf("%-4x",CurrAddr);
		//cout << "#####" _ op _ r1 _ r2 _ tail <<endl;
	}
  int cal(){
    return (op<<26) + (r1<<21) + (r2<<16) + tail;
  }
};
extern vector<InstCode> program;//text segment

//elf文件类，包含elf文件的重要内容，处理elf文件
class ELF_FILE
{
public:

	vector<char> rodata;//rodata segment
	map<string,int> rodata_offset;//offset in .rodata
    vector<Elf32_Rel> reltext;  // relocate text
    vector<Elf32_Sym> symtab; // symbol table
    vector<char> strtab;//strtab segment
    map<string,int> strtab_offset;
    vector<char> shstrtab;// section header str table
    vector<Elf32_Shdr> shdrtab;// section header table
    Elf32_RegInfo reginfo;// register information
    Elf32_Ehdr ehdr;// elf header


public:
    ELF_FILE(){};
    ~ELF_FILE(){};
    /**
     * @Author    Wang_Chuhan    Wang_Zitai
     * @DateTime  2019-01-06
     * @copyright [Copyright (c)2019]
     * 打印调试信息
     */
    void print(){
      cout << "-------------------------rodata--------------------" << endl;
      for(int i = 0; i<rodata.size(); i++){
            if(rodata[i]=='\0')
                putchar('_');
            else
                putchar(rodata[i]);
      }
      puts("");
      cout << "-------------------------reltext--------------------" << endl;
      cout << "offset" << " " << "info" << endl;
      for (int i = 0; i < reltext.size(); ++i)
      {
          Elf32_Rel now_reltext = reltext[i];
          printf("%x\t%x\n", now_reltext.r_offset, now_reltext.r_info);
      }
      cout << "-------------------------symtab--------------------" << endl;
      cout << "Num\tValue\tSize\tInfo\tVis\tNdx\tName\n";
      for (int i = 0; i < symtab.size(); ++i)
      {
            Elf32_Sym sym = symtab[i];
            printf("%d\t%d\t%d\t%x\t%x\t%x\t%d\n", i, sym.st_value, sym.st_size, sym.st_info, sym.st_other, sym.st_shndx, sym.st_name);
      }
      puts("");
      cout << "-------------------------strtab--------------------" << endl;
      for (int i = 0; i < strtab.size(); ++i)
      {
            if(strtab[i]=='\0')
                putchar('_');
            else
                putchar(strtab[i]);
      }
      puts("");

      cout << "------------------------Section Header---------------"<< endl;
      cout << "Nr\tName\tType\tAddr\toff\tSize\tES\tFlg\tLk\tInf  \tAl\n";
      for(int i = 0; i< shdrtab.size(); i++)
      {
          Elf32_Shdr t = shdrtab[i];
          printf("%d\t%d\t%x\t%x\t%x\t%x\t%x\t%x\t%d\t%d  \t%d\n",
                i, t.sh_name, t.sh_type, t.sh_addr, t.sh_offset,
                t.sh_size, t.sh_entsize, t.sh_flags,t.sh_link, t.sh_info, t.sh_addralign
            );
      }
      cout << "----------------Section Header String Table---------------"<< endl;
      for (int i = 0; i < shstrtab.size(); ++i)
      {
            if(shstrtab[i]=='\0')
                putchar('_');
            else
                putchar(shstrtab[i]);
      }
      puts("");
      cout << "-------------RegInfo----------------------" << endl;
      printf("%x ",reginfo.ri_gprmask);
      //cout << reginfo.ri_gprmask _ " ";
      for(int i = 0; i<4 ; i++)
        cout << reginfo.ri_cprmask[4] _ " ";
      cout << reginfo.ri_gp_value << endl;

      cout << "--------------program---------------------" << endl;
      cout << program.size() << endl;
    }


};


#endif //elf_file.h

