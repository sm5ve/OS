typedef unsigned int uint32_t;

extern "C" void higher(char* pageDirectory, char* pageTable1, char* pageTable2){
	char* ptr = (char*)0xB8000;
	//for(;;) *(ptr++) = 'A';
	for(int i = 0; i < 4096; i++){
		pageDirectory[i] = 0;
		pageTable1[i] = 0;
		pageTable2[i] = 0;
	}
	uint32_t* dir = (uint32_t*)pageDirectory;
	dir[0] = (uint32_t)pageTable1 | 0x03;
	dir[768] = (uint32_t)pageTable1 | 0x03;
	dir[1] = (uint32_t)pageTable2 | 0x03;
	dir[769] = (uint32_t)pageTable2 | 0x03;

	uint32_t* tbl1 = (uint32_t*)pageTable1;
	uint32_t* tbl2 = (uint32_t*)pageTable2;
	for(int i = 0; i < 1024; i++){
		tbl1[i] = (i * 4096) | 0x03;
		tbl2[i] = ((i + 1024) * 4096) | 0x03;
	}
	//for(;;) *(ptr++) = 'A';
}
