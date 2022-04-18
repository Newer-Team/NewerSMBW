OUTPUT_FORMAT ("binary")

MEMORY {
	loader_section : ORIGIN = 0x80001800, LENGTH = 0x1800
}

SECTIONS {
	_Z12PAL_OSReportPKcz = 0x8015F870;
	_Z11PAL_OSFatal7GXColorS_PKc = 0x801AF710;
	
	_Z28PAL_DVDConvertPathToEntrynumPKc = 0x801CA7C0;
	_Z15PAL_DVDFastOpeniP9DVDHandle = 0x801CAAD0;
	_Z15PAL_DVDReadPrioP9DVDHandlePviii = 0x801CAC60;
	_Z12PAL_DVDCloseP9DVDHandle = 0x801CAB40;
	
	_Z20PAL_EGG__Heap__allocmiPv = 0x802B8E00;
	_Z19PAL_EGG__Heap__freePvS_ = 0x802B90B0;
	
	
	_Z13NTSC_OSReportPKcz = 0x8015F730;
	_Z12NTSC_OSFatal7GXColorS_PKc = 0x801AF5D0;
	
	_Z29NTSC_DVDConvertPathToEntrynumPKc = 0x801CA680;
	_Z16NTSC_DVDFastOpeniP9DVDHandle = 0x801CA990;
	_Z16NTSC_DVDReadPrioP9DVDHandlePviii = 0x801CAB20;
	_Z13NTSC_DVDCloseP9DVDHandle = 0x801CAA00;
	
	_Z21NTSC_EGG__Heap__allocmiPv = 0x802B8CC0;
	_Z20NTSC_EGG__Heap__freePvS_ = 0x802B8F70;
	
	.text : {
		FILL (0)
		
		__text_start = . ;
		*(.init)
		*(.text)
		*(.ctors)
		*(.dtors)
		*(.rodata)
		*(.sdata)
		*(.sbss)
		*(.fini)
		*(.rodata.*)
		__text_end  = . ;
	}
}
