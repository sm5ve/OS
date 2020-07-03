int max = 30;
char* str = "Hello from Userspace!\n";
char* hello = "Hello from thread ";
int thread_number;

bool isPrime(int x){
	if(x < 2){
		return false;
	}
	for(int i = 2; i * i <= x; i++){
		if(x % i == 0)
			return false;
	}
	return true;
}

void print(int x){
	asm("mov %0, %%eax \n" \
		"int $81" :: "r"(x));
}

void print(char* str){
	asm("mov %0, %%eax \n" \
		"int $80" :: "r"(str));
}

extern "C" int _start(){
	asm("" : "=d"(thread_number) ::);
	asm("outb %0, %1" :: "a"(0x2000), "Nd"(0x604));
	asm("outb %0, %1" :: "a"(0x604), "Nd" (0x2000));
	int i = thread_number * 100000;
	while(true){
		if(isPrime(i)){
			print(i);
		}
		i++;
	}
}
