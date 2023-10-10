#include "ALISS.h"

int main(int argc, char** argv)
{
	int i = 0 ;
	const char * elf_name = 0;
	bool show_help;
    ALISS::memory = (uint8_t *)std::malloc(4 * 1024 * 1024); //4MB size for test

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
		std::cout << "-e elf to load elf file" << std::endl;
		return 0;
	}
	else
	{
		std::cout << elf_name << std::endl;
		if(ALISS::loadElf(elf_name))
        {
		    std::cout << "Load ELF Error" << std::endl;
            return 0;
        }
	}

	while(1)
	{
 	   ALISS::run_pipe();
	}

	return 0;
}
