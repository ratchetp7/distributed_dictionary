struct dict_data{
	string word<>;
	string meaning<>;
	int flag;
};
program DICTIONARY_PROG { 
	version DICTIONARY_VERS { 
		dict_data OPERATION_EXECUTE(dict_data) = 1; 
	} = 1; 
} = 0x24123112;
