#include "main.h"

void ALISS::loadElf(const char* filename)
{
    // ELF loader function
    std::ifstream file(filename, std::ios::binary);

    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    // Read the ELF header
    Elf64_Ehdr elfHeader;
    file.read(reinterpret_cast<char*>(&elfHeader), sizeof(Elf64_Ehdr));

    // Check ELF magic number
    if (std::memcmp(elfHeader.e_ident, ELFMAG, SELFMAG) != 0) {
        std::cerr << "Not a valid ELF file: " << filename << std::endl;
        return;
    }

    // Check ELF class (32-bit or 64-bit)
    if (elfHeader.e_ident[EI_CLASS] != ELFCLASS64) {
        std::cerr << "Only 64-bit ELF files are supported: " << filename << std::endl;
        return;
    }

    // Check ELF data encoding (little-endian or big-endian)
    if (elfHeader.e_ident[EI_DATA] != ELFDATA2LSB) {
        std::cerr << "Only little-endian ELF files are supported: " << filename << std::endl;
        return;
    }

    // Get the entry point address
    Elf64_Addr entryPoint = elfHeader.e_entry;
    ALISS::pc = entryPoint;

    // Add more code here to load and work with program segments, sections, etc.

    file.close();
	return;
}

int main(int argc, char** argv)
{
	int i = 0 ;
	const char * elf_name = 0;
	bool show_help;

	for( i = 1; i < argc; i++ )
	{
		const char * param = argv[i];

		if( param[0] == '-')
		{
			switch( param[1] )
			{
				case 'e': elf_name = (++i < argc) ? argv[i] : 0; break;
				case 'h': show_help = 1; break;
			default:
					std::cout  << "No command " << param[1] << std::endl;
					show_help = 1;
				break;
			}
		}
		else
		{
			show_help = 1;
			break;
		}

		param++;
	}

	if(show_help)
	{
		std::cout << "Help Help" << std::endl;
		return 0;
	}
	else
	{
		std::cout << elf_name << std::endl;
		ALISS::loadElf(elf_name);
	}

	return 0;
}
