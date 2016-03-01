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
sbit lcden=P3^4;	//Һ��ʹ�ܶ�
sbit lcdrs=P3^5;	//Һ����������ѡ���
sbit dula=P2^6;		//����U1�������������
sbit wela=P2^7;		//����U2�������������
uchar num;
int STATU=0;	   //״̬�� ��־����Ϊ�� ���ΪС�� Ĭ��0Ϊ����״̬
uint op1=0,op2=0;  //op1->��һ��������	op2->�ڶ���������	
uchar opc=0,pos=0; //opc->�����  ops->λ��
long int res=0;			//��������ֵΪ0
int res2=0;			//С������
uchar lastkey;

/************************
	��ʱ����
************************/
void delay(uint z){
	uint x,y;
	for(x=z;x>0;x--)
		for(y=110;y>0;y--);
}


/************************
	Һ��д����
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
	Һ��д����
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
	Һ����ʼ��
************************/
void init(){
	dula=0;
	wela=0;
	lcden=0;
	write_com(0x38);//����16X2��ʾ,5X7����,8λ���ݽӿ�
	write_com(0x0f);//���ÿ���ʾ������ʾ���
	write_com(0x06);//дһ���ַ����ַָ���1
	write_com(0x01);//��ʾ���㣬����ָ������
}


/***********************
		Һ����ʾ�����ַ�
		x->λ��
		y->0->��һ��
		y->1->�ڶ���
		ch->Ҫд����ַ���һ��ֻ��д��һ���ַ�����
************************/
void display1602c(uchar x,uchar y,uchar ch){ 
  //x=0-15,y=0-1;
  uchar Addr;
  Addr=x+(y*0x40);	 
  write_com(0x80+Addr);
  if(ch!='='){write_data(ch);}
  
}

//������� ��û���õ�
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
		Һ����ʾ������
		��ȡ�����ÿһλ�������ʾ
*******************************************/
uchar display1602i(uchar x,uchar y,int num){ 


/**************2016.3.1Һ����ʾǰ�ж��ǹ�����**********************/


  if(STATU==1){ /*����Ϊ0����ʾERROR*/
  		uchar code info[6]="ERROR";
		int i=0;
		write_com(0x80+(y*0x40));
		for(i=0;i<5;i++){	
			write_data(info[i]); //�ڼ�����ǰ��ʾ''
		}
	
		STATU=0;
  }
	else if(STATU==2){  /*���ΪС��*/
		uchar Addr,a[6],t=0,i,flag=0;
		if(num==0){
			a[t++]='0';
		} 
		while(num!=0){//��ȡ��������ÿһλ������ 	
			a[t++]=num%10+'0';
			num=num/10;
		} 
		if(flag){//���������Ϊ��������Ӹ���'-'
			a[t++]='-'; 
		}
	
		Addr=x+(y*0x40);
		write_com(0x80+(y*0x40));	
		write_data('='); //�ڼ�����ǰ��ʾ'='
		write_com(0x80+Addr);
		for(i=0;i<t;i++){	 
			write_data(a[t-1-i]);
			tb[i]=a[t-1-i];
		}
		write_data('.');
		
		for(i=0;i<3;i++){//��ȡС�����ֵ�ÿһλ�����ֲ���ʾ....			
			temp[i]=res2%10+'0';								  
			res2=res2/10;	
		}

		for(i=0;i<3;i++){
			write_data(temp[2-i]);
		}
		STATU=0;
	}
	else{/*���Ϊ��������*/
		uchar Addr,a[6],t=0,i,flag=0;
		if(num==0){
			a[t++]='0';
		}   
		if (num<0){//���������Ϊ����������ת�������� 
			num=-num; 
			flag=1;   
		}
		while(num!=0){//��ȡ��������ÿһλ������	
		a[t++]=num%10+'0';
		num=num/10;
		} 
		if(flag){//���������Ϊ��������Ӹ���'-'
			a[t++]='-'; 
		}
		Addr=x+(y*0x40);
  		write_com(0x80+(y*0x40));	
		write_data('='); //�ڼ�����ǰ��ʾ'='

		write_com(0x80+Addr);
		for(i=0;i<t;i++){	 
			write_data(a[t-1-i]);
			tb[i]=a[t-1-i];
  		}
	return (x+t);
	}
}


/***********************
		��ʱ����
************************/
void delayms(uint xms){
	uint i,j;
	for(i=xms;i>0;i--)		      //i=xms����ʱԼxms����
		for(j=110;j>0;j--);
}



/************************
	����ɨ�躯��
	���ذ���4*4��������һ������0--15��
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
  	ɨ�������ȫ������
	1.�������֣�
		��1�������һ�������ǡ�=��,�������������ݣ����¼���
		��2���ж��Ƿ��Ѿ������������δ���룬��ȡΪ��һ�����������ȡΪ�ڶ�����
	2.�����������
		���������ֵ��opc
	3.����Ⱥţ�
		�������������������㣬�ó��������ʾ�ڵڶ���
*******************************************************************************************/
void key_proc(){ 
  uchar i,key;
  if(tb[0]=='/'||tb[0]=='*'){
  	STATU=1;
  }
  for(i=0;i<pos;i++){
    key=tb[i];
    if((key>='0')&&(key<='9')){	
    	if(lastkey=='='){ //�����һ�ΰ�����'='	���������������ݣ����¼���
		   write_com(0x01);
	       op1=0;op2=0;opc=0;res=0;pos=0;
     	}
    
   		if(opc==0){  //��һ��������
			op1=op1*10+(key-'0');
		}
		else{ //�ڶ���������
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
			if(op2==0){/*�������Ϊ0��״̬�븳ֵΪ1*/
	   	   		STATU=1;
		   		break;
	   		}
	   		else if(op1%op2!=0){/*������ΪС����״̬�븳ֵΪ2*/
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
	init();	 //Һ����ʼ��
	while(1){ 
		key=matrixkeyscan();  //����ɨ��
	    if(key<16){ 
		key=table[key];	    //����ǰ���¼�λת���ɶ�Ӧ���ַ�

/************����'C'������Ѿ��������ݣ���ǰɾ��һλ***************/		
	    	if(key=='C'){  
				if(pos>0){ //pos����0�����Ѿ���������
				pos--;
		    	display1602c(pos,0,' '); //��posλ��д��' '����ɾ��
		    	write_com(0x10);//�������ƽ��һλ��ACֵ��1
		  		}
			}
/********************  ����'='  *******************************/
			else if(key=='='){ //���¡�=��
				tb[pos]=key;
		  		display1602c(pos++,0,key);	//��ʾ�Ⱥ�
		  		key_proc();		//���㲢��ʾ���
			}

/********************  �������ּ��������  *******************************/
			else{
				if(lastkey=='='){//�����һ���Ѿ�����=������	
					if((key>='0')&&(key<='9')){//�ٰ������֣���һ�����������¼���
            			write_com(0x01);
			    		op1=0;op2=0;opc=0;res=0;pos=0;			
					}
					else{//�ٰ��²���������������ʾ��һ��������
            			write_com(0x01);
						pos=0;
						pos=display1602i(pos,0,res);
			    		op1=0;op2=0;opc=0;res=0;
				
					}	
		  		}  

		  		tb[pos]=key;
		  		display1602c(pos++,0,key);	//��ʾ���µ��ַ�
			}  
		lastkey=key;
		}
	}
}

