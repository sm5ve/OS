int x = 2;

extern "C" int _start(){
	for(int i = 0; i < x; i++)
		asm("int $80");
	return 0;
}
