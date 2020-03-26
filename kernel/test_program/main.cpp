int max = 30;
char* str = "Hello world!\n";

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
	for(int i = 0; i < max; i++){
		if(isPrime(i)){
			print(i);
		}
	}
	print(str);
	for(;;);
	return 0;
}
