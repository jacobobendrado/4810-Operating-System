- ## What does linker.ld do?
	- ### Resources
		- [Using LD, the GNU linker](https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_chapter/ld_3.html)
	- ### [Linker Scripts](https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_mono/ld.html#SEC6)
		- "When developing user-space programs, your toolchain ships with default scripts for linking such programs. However, these are unsuitable for kernel development and you need to provide your own customized linker script." (from [OSDev](https://wiki.osdev.org/Bare_Bones#Linking_the_Kernel))
		- LD accepts Linker Command Language files written in a superset of AT&T’s Link Editor Command Language syntax.
		- **SECTIONS** “The most fundamental command of the `ld` command language is the `SECTIONS` command (see section [Specifying Output Sections](https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_mono/ld.html#SEC17)). Every meaningful command script must have a `SECTIONS` command: it specifies a ‘picture‘ of the output file's layout, in varying degrees of detail. No other command is required in all cases.”
			- only one `SECTIONS` command is permitted. you can have as many statements within the `SECTIONS` as you would like.
			- statements can do one of three things
				- describe the placement of a named output section and which input sections go into it.
				  ```
				  SECTIONS {
				  	section_name : {
				      	contents
				      }
				      
				      section2 : {
				      	contents
				      }
				  }
				  ```
				- define an entry point
				- assign a value to a symble
			- valid section names are determined by the output format (IE: a.out only allows `.text`, `.data`, and `.bss`) there is a special name, `/DISCARD/`which will… discard anything placed within it. (they will not appear in the final output)
			- the contents of a section can be specified in a number of ways:
				- filename ` .data : { bingus.o, bongus.o }`
					- the whole file
				- filename( section ) `.data : { input_file.o( .data ) }`
				- filename ( section, section, … ) `.text : { input_file.o( .multiboot, .text ) }`
					- specified sections from specified files
				- *(section) `.text : { *(.text ) }`
				- *(section, section, …) `.text : { *(.multiboot, .text) }`
					- the section name(s) from taken from all input files
				- filename( COMMON ) `.text : { bingus.o( COMMON  ) }`
				- *( COMMON ) `.text : { *( COMMON  ) }`
					- where to put uninitialized data from one or all files. `ld` permits you to refer to all uninitialized data as though it were in an input-file section named `COMMON` regardless of the input file’s format
				- In the following example, the command script arranges the output file into three consecutive sections, named `.text`, `.data`, and `.bss`, taking the input for each from the correspondingly named sections of all the input files:
				  ```
				  SECTIONS { 
				  	.text : { 
				      	*(.text)
				  	}
				  	
				      .data : { 
				      	*(.data) 
				      } 
				  	
				      .bss :  { 
				      	*(.bss) 
				      	*(COMMON)
				      } 
				  }
				  ```
			- `ALIGN(n)`: returns the result of the current location counter, `.`, aligned to the next `n` boundary. `n` must be a power of two. note: `ALIGN` does not change the value of the location counter, it just performs arithmetic on it.
		- There is a special linker variable `.` which always contains the current output location counter. this value **can** be assigned, but **cannot** be moved backwards. enabling you to leave “holes” in your final output file. it can **only** be used within a `SECTIONS` command
			- In the following example, `file1` is located at the beginning of the output section, then there is a 1000 byte gap. Then `file2`appears, also with a 1000 byte gap following before `file3` is loaded. The notation `= 0x1234' specifies what data to write in the gaps
			  ```
			  SECTIONS
			  {
			    output :
			    {
			    file1(.text)
			    . = . + 1000;
			    file2(.text)
			    . += 1000;
			    file3(.text)
			    } = 0x1234;
			  }
			  ```