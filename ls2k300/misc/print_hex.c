/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */

extern void printk(const char *fmt, ...);

/*
 * print buffer as hex char
 */
void print_hex(char *p, int size)
{
	char hexval[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

	int i, j;
	char ch_hi, ch_lo;
#if __loongarch64
	unsigned long paddr = (unsigned long)p;
#else
	unsigned int paddr = (unsigned int)p;
#endif
	j = paddr % 4;

	for (i = 1; i <= size + j; i++)
	{
		if (i <= j)
		{
			printk("  ");
		}
		else
		{
			ch_hi = hexval[(((unsigned char)p[0] >> 4) & 0x0F)];
			ch_lo = hexval[((unsigned char)p[0] & 0x0F)];

			printk("%c%c", ch_hi, ch_lo);

			p++;
		}

		if ((i % 4) == 0) printk(" ");
		if ((i % 32) == 0) printk("\r\n");
	}

	printk("\r\n");
}
