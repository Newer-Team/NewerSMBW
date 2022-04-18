#ifndef NSMBWVER_H
#define NSMBWVER_H

enum NSMBWVer
{
	pal = 0,
	pal2 = 1,
	ntsc = 2,
	ntsc2 = 3,
	jpn = 4,
	jpn2 = 5,
	kor = 6,
	twn = 7
};

NSMBWVer getNsmbwVer();

#endif