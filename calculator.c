#include<reg52.h>
#define uchar unsigned char
#define uint unsigned  int
uchar code table[]={
	'7','8','9','+',
	'4','5','6','-',
	'1','2','3','*',
	'C','0','=','/'
};
uchar tb[12];
uchar temp[3];
sbit lcden=P3^4;	//液晶使能端
sbit lcdrs=P3^5;	//液晶数据命令选择端
sbit dula=P2^6;		//申明U1锁存器的锁存端
sbit wela=P2^7;		//申明U2锁存器的锁存端
uchar num;
int STATU=0;	   //状态码 标志除数为〇 结果为小数 默认0为正常状态
uint op1=0,op2=0;  //op1->第一个输入数	op2->第二个输入数	
uchar opc=0,pos=0; //opc->运算符  ops->位置
long int res=0;			//计算结果初值为0
int res2=0;			//小数部分
uchar lastkey;

/************************
	延时函数
************************/
void delay(uint z){
	uint x,y;
	for(x=z;x>0;x--)
		for(y=110;y>0;y--);
}


/************************
	液晶写命令
************************/
void write_com(uchar com){
	lcdrs=0;
	P0=com;
	delay(5);
	lcden=1;
	delay(5);
	lcden=0;
}

/***********************
	液晶写数据
************************/
void write_data(uchar date){
	lcdrs=1;
	P0=date;
	delay(5);
	lcden=1;
	delay(5);
	lcden=0;
}


/***********************
	液晶初始化
************************/
void init(){
	dula=0;
	wela=0;
	lcden=0;
	write_com(0x38);//设置16X2显示,5X7点阵,8位数据接口
	write_com(0x0f);//设置开显示，不显示光标
	write_com(0x06);//写一个字符后地址指针加1
	write_com(0x01);//显示清零，数据指针清零
}


/***********************
		液晶显示输入字符
		x->位置
		y->0->第一行
		y->1->第二行
		ch->要写入的字符（一次只能写入一个字符？）
************************/
void display1602c(uchar x,uchar y,uchar ch){ 
  //x=0-15,y=0-1;
  uchar Addr;
  Addr=x+(y*0x40);	 
  write_com(0x80+Addr);
  if(ch!='='){write_data(ch);}
  
}

//这个函数 并没有用到
void display1602s(uchar x,uchar y,uchar *s){ 
  //x=0-15,y=0-1;
  uchar Addr;
  Addr=x+(y*0x40);	 
  write_com(0x80+Addr);
  while(*s!=0){ 
    write_data(*s);
    s++;
  }
}


/*******************************************
		液晶显示计算结果
		获取结果的每一位并输出显示
*******************************************/
uchar display1602i(uchar x,uchar y,int num){ 


/**************2016.3.1液晶显示前判断是够出错**********************/


  if(STATU==1){ /*除数为0，显示ERROR*/
  		uchar code info[6]="ERROR";
		int i=0;
		write_com(0x80+(y*0x40));
		for(i=0;i<5;i++){	
			write_data(info[i]); //在计算结果前显示''
		}
	
		STATU=0;
  }
	else if(STATU==2){  /*结果为小数*/
		uchar Addr,a[6],t=0,i,flag=0;
		if(num==0){
			a[t++]='0';
		} 
		while(num!=0){//读取计算结果的每一位的数字 	
			a[t++]=num%10+'0';
			num=num/10;
		} 
		if(flag){//如果计算结果为负数，添加负号'-'
			a[t++]='-'; 
		}
	
		Addr=x+(y*0x40);
		write_com(0x80+(y*0x40));	
		write_data('='); //在计算结果前显示'='
		write_com(0x80+Addr);
		for(i=0;i<t;i++){	 
			write_data(a[t-1-i]);
			tb[i]=a[t-1-i];
		}
		write_data('.');
		
		for(i=0;i<3;i++){//读取小数部分的每一位的数字并显示....			
			temp[i]=res2%10+'0';								  
			res2=res2/10;	
		}

		for(i=0;i<3;i++){
			write_data(temp[2-i]);
		}
		STATU=0;
	}
	else{/*结果为正常数据*/
		uchar Addr,a[6],t=0,i,flag=0;
		if(num==0){
			a[t++]='0';
		}   
		if (num<0){//如果计算结果为负数，将它转换成正数 
			num=-num; 
			flag=1;   
		}
		while(num!=0){//读取计算结果的每一位的数字	
		a[t++]=num%10+'0';
		num=num/10;
		} 
		if(flag){//如果计算结果为负数，添加负号'-'
			a[t++]='-'; 
		}
		Addr=x+(y*0x40);
  		write_com(0x80+(y*0x40));	
		write_data('='); //在计算结果前显示'='

		write_com(0x80+Addr);
		for(i=0;i<t;i++){	 
			write_data(a[t-1-i]);
			tb[i]=a[t-1-i];
  		}
	return (x+t);
	}
}


/***********************
		延时函数
************************/
void delayms(uint xms){
	uint i,j;
	for(i=xms;i>0;i--)		      //i=xms即延时约xms毫秒
		for(j=110;j>0;j--);
}



/************************
	键盘扫描函数
	返回按下4*4键盘中哪一个键（0--15）
*************************/
uchar matrixkeyscan(){
  uchar i,temp,key=16,line=0xfe;
  for(i=0;i<4;i++){ 
    P1=line;   //0xfe,0xfd,0xfb,0xf7
    temp=P1;//temp=0xfe;
    temp=temp&0xf0;	//temp=0xf0
    if(temp!=0xf0){
      delayms(10);
	  temp=P1;
	  temp=temp&0xf0;
      if(temp!=0xf0){
        temp=P1;
        switch(temp&0xf0){
          case 0xe0:   //0xee,0xed,0xeb,0xe7;
               key=4*i;    //0,4,8,12
               break;
          case 0xd0:  //0xdd,0xdb,0xd7;
               key=4*i+1;
               break;
          case 0xb0:  //0xbd,0xbb,0xb7;
               key=4*i+2;
               break;
          case 0x70:  //0x7d,0x7b,0x77;
               key=4*i+3;
               break;
         }
         while(temp!=0xf0){
           temp=P1;
           temp=temp&0xf0;
         }
        //display(key);
      }
    }
	line=(line<<1)+1;	
  }
  return key;
}



/*****************************************************************************************
  	扫描输入的全部数据
	1.输入数字：
		（1）如果上一步按的是“=”,清屏，重置数据，重新计算
		（2）判断是否已经输入运算符，未输入，读取为第一个数，否则读取为第二个数
	2.输入运算符：
		将运算符赋值给opc
	3.输入等号：
		对两个操作数进行运算，得出结果并显示在第二行
*******************************************************************************************/
void key_proc(){ 
  uchar i,key;
  if(tb[0]=='/'||tb[0]=='*'){
  	STATU=1;
  }
  for(i=0;i<pos;i++){
    key=tb[i];
    if((key>='0')&&(key<='9')){	
    	if(lastkey=='='){ //如果上一次按的是'='	，清屏，重置数据，重新计算
		   write_com(0x01);
	       op1=0;op2=0;opc=0;res=0;pos=0;
     	}
    
   		if(opc==0){  //第一个操作数
			op1=op1*10+(key-'0');
		}
		else{ //第二个操作数
			 op2=op2*10+(key-'0');
		}
 	 }
 	 else if((key=='+')||(key=='-')||(key=='*')||(key=='/')){	 
	 	opc=key;
  	 }
	else if(key=='='){
		switch(opc){
		case '+':res=op1+op2;break;
		case '-':res=op1-op2;break;
		case '*':res=op1*op2;break;
		case '/': 
			if(op2==0){/*如果除数为0，状态码赋值为1*/
	   	   		STATU=1;
		   		break;
	   		}
	   		else if(op1%op2!=0){/*如果结果为小数，状态码赋值为2*/
				STATU=2;
				res=op1/op2;
				res2=op1%op2*1000/op2;
				break;
	   		}
	   		else{
				res=op1/op2;break;
			}		
		}	
		display1602i(1,1,res);
	}
 }
}

void main(){	
	uchar key;
	init();	 //液晶初始化
	while(1){ 
		key=matrixkeyscan();  //键盘扫描
	    if(key<16){ 
		key=table[key];	    //将当前按下键位转换成对应的字符

/************按下'C'，如果已经输入数据，向前删除一位***************/		
	    	if(key=='C'){  
				if(pos>0){ //pos大于0，即已经输入数据
				pos--;
		    	display1602c(pos,0,' '); //在pos位置写入' '，即删除
		    	write_com(0x10);//光标向左平移一位，AC值减1
		  		}
			}
/********************  按下'='  *******************************/
			else if(key=='='){ //按下‘=’
				tb[pos]=key;
		  		display1602c(pos++,0,key);	//显示等号
		  		key_proc();		//计算并显示结果
			}

/********************  按下数字键或操作符  *******************************/
			else{
				if(lastkey=='='){//如果上一步已经按了=计算结果	
					if((key>='0')&&(key<='9')){//再按下数字，第一行清屏，重新计算
            			write_com(0x01);
			    		op1=0;op2=0;opc=0;res=0;pos=0;			
					}
					else{//再按下操作符，清屏，显示上一步计算结果
            			write_com(0x01);
						pos=0;
						pos=display1602i(pos,0,res);
			    		op1=0;op2=0;opc=0;res=0;
				
					}	
		  		}  

		  		tb[pos]=key;
		  		display1602c(pos++,0,key);	//显示按下的字符
			}  
		lastkey=key;
		}
	}
}

