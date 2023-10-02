#include "ALISS.h"

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
		if(ALISS::loadElf(elf_name))
        {
		    std::cout << "Load ELF Error" << std::endl;
            return 0;
        }
	}

    ALISS::run_pipe();

	return 0;
}
