int max = 30;

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

extern "C" int _start(){
	for(int i = 0; i < max; i++){
		if(isPrime(i)){
			asm("mov %0, %%eax \n" \
				"int $81" :: "r"(i));
		}
	}
	for(;;);
	return 0;
}
