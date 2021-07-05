#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <list.h>

//引入list.h头文件，list_head操作都定义在里面

#define length_Prep 15	 //每个虚词的长度 
#define MAX 30 			//单词存放的长度 

//虚词数组
char prep[][length_Prep]= {{"of"},{"to"},{"in"},{"and"},{"as"},{"from"},{"for"},{"with"},{"that"},{"have"},
	{"by"},{"on"},{"upon"},{"about"},{"above"},{"across"},{"among"},{"ahead"},{"after"},{"a"},
	{"an"},{"although"},{"at"},{"also"},{"along"},{"around"},{"always"},{"away"},{"any"},{"up"},
	{"under"},{"until"},{"before"},{"between"},{"beyond"},{"behind"},{"because"},{"what"},{"when"},{"would"},
	{"could"},{"who"},{"whom"},{"whose"},{"which"},{"where"},{"why"},{"without"},{"whether"},{"down"},
	{"during"},{"despite"},{"over"},{"off"},{"only"},{"other"},{"out"},{"than"},{"the"},{"then"},
	{"through"},{"throughout"},{"that"},{"these"},{"this"},{"those"},{"there"},{"therefore"},{"till"},{"some"},
	{"such"},{"since"},{"so"},{"can"},{"many"},{"much"},{"more"},{"may"},{"might"},{"must"},
	{"ever"},{"even"},{"every"},{"each"}
};

FILE *fp;
char word[100];			//单词存放于此
char oneWord;			//记录每一个字母
int script = 0;			//记录word数组下标
int i;
int flag=0;				//不是虚词则为1
int repeat=0;
int symbol=0;				//判断是否是" ' " ," - "的标记 
int prep_Size=sizeof(prep)/sizeof(prep[0]);		//统计虚词数组中每个元素的空间大小 
int totalWord = 0;		//统计全文单词（无虚词） 


struct list_head head;
struct list_head *p;


// 定义该结构体存储单词及计数器
typedef struct stroe_Word {
	char words[MAX];		//存放单词
	int count;			//统计单词出现的次数
	struct list_head list;
} stroe_Word;

//-------------------------------------------------------------------------------

void file_open() {
	if((fp=fopen("Linux_of_test.txt","r"))==NULL) {
		printf("无法打开此文件!\n");
		exit(0);
	}
}

//判断传来的单词是否为虚词 
int checkPrep(char word[]){
	for(i=0; i<prep_Size; i++) 
	{			
	   if(strcmp(word,prep[i])==0) {
	   	flag=1;
	   	break;
	   }
	}
	return flag;
}

//检查单词是否在链表中有出现过 
int checkRepeat(struct list_head *head , struct list_head *p, char word[] ,int repeat) {
	if(!list_empty(head)) {
		list_for_each(p,head) {
			stroe_Word *node = list_entry(p,stroe_Word,list);
			if (strcmp(word , node->words)==0) {
				repeat=1;
				node->count++;//不插入，计数器累加
				break; 
			}
		}
	}
	return repeat;
}

//判断是否为''情况，如果是的话则将后面的'变成\0处理 
int checkSymbol(char oneWord , char word[]){
	
	if(oneWord == '\'') {
		if(symbol>0) {
			word[script] = '\0';
			flag = checkPrep(word); 
			checkRepeat(&head,&p,word,repeat);
		} else {
			symbol++;
			return symbol;
		}
	}
	symbol = 0;		
	return symbol;
}

//若不是虚词且无重复则肯定是新单词，添加新的链表节点 
void addWord(struct list_head *head , char word[] , int flag , int repeat) {
	stroe_Word *newWord;
	if (repeat==0 && flag==0) {
		newWord = (stroe_Word *)malloc(sizeof(stroe_Word));
		strcpy( newWord->words , word );
		newWord->count=1;
		list_add_tail( &(newWord->list) , head );
	}
}

//一个一个字母的进行大小写转换，并进行虚词，相同词的比较，若前两者的比较都不是则增加新单词 
void addLetter(char oneWord , char word[]){	
	//将大写转小写，以便于相同词，只是有大小写区分的统计 
	if ((oneWord>='a'&&oneWord<='z')||(oneWord>='A'&&oneWord<='Z')) {
			if(oneWord>='A'&&oneWord<='Z') {
				word[script++]=oneWord+32;			
			} else {
				word[script++]=oneWord;
			}
		}
	//到单词的结尾了
	else if (script > 0) {						
		word[script] = '\0';
		totalWord++;				//当单词到了结尾后，totalWord++，即总单词数增加 
		//检查是否是虚词 
		flag = checkPrep(word);			
		// 链表遍历，比较链表中的节点值与当前单词
		repeat = checkRepeat(&head,&p,word,repeat);
		// 如果链表中没有当前单词，在链表末尾插入节点
		addWord(&head,word,flag,repeat);
		script=0;
	}
}

//将传来的单词与链表中的单词相比，如果有相同的则将传来的单词的计数值加至原单词计数值上 
int changeCount(struct list_head *p , struct list_head *head , char temp[] , stroe_Word *q){
	list_for_each(p,head){
		stroe_Word *node = list_entry(p,stroe_Word,list);
		if( strcmp(node->words , temp)==0 ){
			node->count = node->count + q->count;
			return 1;
		} 
	}
	return 0;
}

//判断通常情况下的复数，并将其与一般情况的单词一起统计 
int isWordEnd_S(stroe_Word *q , struct list_head *p , struct list_head *head){
	char temp[MAX];
	char *val = temp;
	strcpy(temp , q->words);
	while(*val)val++;
	val--;							//在while循环后，val会指向结尾的\0，所以要--向前移动一位 
	if(*val && *val!='s')return 0;	//若该单词不是s结尾，则不是复数形式 
	if(*(val-1)) *val='\0';			//若上条if不成立，且确定val的前一位的字母是存在的，则将该位置的s替换为结束符 
	else return 0;					
	if(changeCount(p,head,temp,q))return 1;//通过遍历数组来寻找是否在文中出现过去除s后的该单词，如果有那就证明该单词去s便是原型 
	val--;						//若通过遍历后发现没有，则怀疑是es或ies的情况，所以向前移一位 
	if(*val && *val!='e')return 0;	//若不为e的话该单词则不为复数 
	*val='\0';						//上条if不成立，证明该元素为e，则将该位置的e替换为结束符
	if(changeCount(p,head,temp,q))return 1;
	val--;							//若通过遍历后发现没有，则只有改y为i+es的情况，所以向前移一位 
	if(*val && *val!='i') return 0;	//若不为i的话该单词则不为复数 
	*val='y';						//该i为y还原单词 
	val++;
	*val='\0';
	if(changeCount(p,head,temp,q))return 1;
	return 0;
}

//判断通常情况下的过去式单词，并将其与一般情况的单词一起统计
int isWordEnd_D(stroe_Word *q , struct list_head *p , struct list_head *head){
	char temp[MAX];
	char *val = temp;
	strcpy(temp , q->words);
	while(*val)val++;
	val--;							//在while循环后，val会指向结尾的\0，所以要--向前移动一位 
	if(*val && *val!='d')return 0;	//若该单词不是d结尾，则不是复数形式 
	if(*(val-1)) *val='\0';			//若上条if不成立，且确定val的前一位的字母是存在的，则将该位置的s替换为结束符 
	else return 0;					
	if(changeCount(p,head,temp,q))return 1;	//通过遍历数组来寻找是否在文中出现过去除s后的该单词，如果有那就证明该单词去s便是原型 
	val--;							//若通过遍历后发现没有，则怀疑是es或ies的情况，所以向前移一位 
	if(*val && val!='e')return 0;	//若不为e的话该单词则不为复数 
	*val='\0';						//上条if不成立，证明该元素为e，则将该位置的e替换为结束符
	if(changeCount(p,head,temp,q))return 1;
	return 0;
}

//通过count值进行排序（选择）
void choose_Sort(struct list_head *head) {
	int max = 0;
	int temporary;		//count交换值时的中间变量 
	char temp[MAX];
	struct list_head *tmp=head->next;	//tmp指针会从第一个一直移动至最后一个结点 
	struct list_head *maxPoint;			//maxPoint用于指向此轮循环最大值所在位置
	
	while(tmp->next != head) {
		struct list_head *tmpNext = tmp->next;
		stroe_Word *node1 = list_entry(tmp,stroe_Word,list);
		max = node1->count;
		maxPoint = tmp;
		
		while(tmpNext!=head) {
			stroe_Word *node2 = list_entry(tmpNext,stroe_Word,list);
			if(max < node2->count) {
				max = node2->count;//更新最大值
				maxPoint = tmpNext;
			}
			tmpNext = tmpNext->next;
		}
		
		//改变结点内值的内容
		if(max!=node1->count) { 
			stroe_Word *node3 = list_entry(maxPoint,stroe_Word,list);
			temporary = node1->count;
			node1->count = node3->count;
			node3->count = temporary;

			strcpy(temp , node1->words);
			strcpy(node1->words , node3->words);
			strcpy(node3->words , temp);
		}
		tmp = tmp->next;
	}
}

//统计单词频率总数
int countNumber(struct list_head *head , struct list_head *p){
	int countNum = 0;
	list_for_each(p,head){
		stroe_Word *node = list_entry(p,stroe_Word,list);
		countNum +=  node->count;
	}
	return countNum; 
}

//显示格式 
void showWord(struct list_head *head , struct list_head *p){
	int j=50;			//默认输出50个单词 
	float countNum = countNumber(head,p);	//countNum为单词的总数（无虚词） 
	struct list_head *q;	//用于指向list_head链表以控制循环 
	q = head->next;
	
	printf("----------------------------------------------------------------------------------------------------------\n"); 
	printf("||		                     本文的总单词数为(含虚词)： %d					 |\n" , totalWord);
	printf("----------------------------------------------------------------------------------------------------------\n");
	printf("||		                     本文的总单词数为(不含虚词)： %d					 |\n" , (int)countNum); 
	printf("----------------------------------------------------------------------------------------------------------\n");
	printf("||        单词(前50个)       ||   出现的个数    ||  出现的频率(不计算虚词)(%%)  ||  出现的频率(全文)(%%)   |\n");
	printf("----------------------------------------------------------------------------------------------------------\n");
	printf("----------------------------------------------------------------------------------------------------------\n");
	
	while(q->next != head){		//list_head列表的结尾不是NULL，而是链回到head 
		if(j == 0)break;
		else{
			stroe_Word *node = list_entry(q,stroe_Word,list);
			printf("||%-25s  ||       %-8d  ||         %5.2f %%  	       ||     	 %5.2f %%         |\n" , 
					node->words , node->count ,((float)(node->count)/countNum)*100,((float)(node->count)/(float)totalWord)*100);
			printf("----------------------------------------------------------------------------------------------------------\n");
			j--;
			q = q->next;
		}		
	}
}

//---------------------------------------------------------------------------


int main() {
	
	INIT_LIST_HEAD(&head);  //初始化
	file_open();

	while (!feof(fp)) {
		repeat=0;
		flag=0;
		//逐个获取字符
		oneWord = fgetc(fp);			
		//检查是否为特殊字符	
		if( checkSymbol(oneWord,word)>0 ) continue;
		//按字母检索检测是否为单词，统计单词是否由出现过，并在list_head后加入节点 
		addLetter(oneWord,word);
	}
	// 对文件进行操作，关闭文件
	fclose(fp);
	//对文章中的复数、过去式的单词进行判断 
	list_for_each(p,&head){
		stroe_Word *node = list_entry(p,stroe_Word,list);
		if( (isWordEnd_S(node , p , &head))==1 || (isWordEnd_D(node , p , &head))==1) 
			list_del(p);
	}
	
	//统计文中去除虚数后的单词数 
	countNumber(&head,p);
	
	//调用排序算法
	choose_Sort(&head); 
	//遍历链表，打印结果
	showWord(&head , &p);
	
	return 0;
}
