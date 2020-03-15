typedef unsigned int uint32_t;

extern "C" void higher(char* pageDirectory, char* pageTable1, char* pageTable2, char* pageTable3, char* pageTable4){
	//char* ptr = (char*)0xB8000;
	//for(;;) *(ptr++) = 'A';
	for(int i = 0; i < 4096; i++){
		pageDirectory[i] = 0;
	}
	uint32_t* dir = (uint32_t*)pageDirectory;
	dir[0] = (uint32_t)pageTable1 | 0x03;
	dir[768] = (uint32_t)pageTable1 | 0x03;
	dir[1] = (uint32_t)pageTable2 | 0x03;
	dir[769] = (uint32_t)pageTable2 | 0x03;
	dir[2] = (uint32_t)pageTable3 | 0x03;
	dir[770] = (uint32_t)pageTable3 | 0x03;
	dir[3] = (uint32_t)pageTable4 | 0x03;
	dir[771] = (uint32_t)pageTable4 | 0x03;
	
	uint32_t* tbl1 = (uint32_t*)pageTable1;
	uint32_t* tbl2 = (uint32_t*)pageTable2;
	uint32_t* tbl3 = (uint32_t*)pageTable3;
	uint32_t* tbl4 = (uint32_t*)pageTable4;
	for(int i = 0; i < 1024; i++){
		tbl1[i] = (i * 4096) | 0x03;
		tbl2[i] = ((i + 1024) * 4096) | 0x03;
		tbl3[i] = ((i + 2 * 1024) * 4096) | 0x03;
		tbl4[i] = ((i + 3 * 1024) * 4096) | 0x03;
	}
	//for(;;) *(ptr++) = 'A';
}
