all:program

program: program.S
	aarch64-linux-gnu-as -o program.o program.S
	aarch64-linux-gnu-ld -o program program.o

cpio:
# -o means create, -H means format and in this class we use New ASCII Format  
	find . | cpio -o -H newc > ../initramfs.cpio
clean:
	rm program.o program