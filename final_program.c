#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#define MAXFIELDS 100 // for now
#define MAXINPUTLENGTH 5000
#define MAXLENOFFIELDNAMES 50
#define MAXLENOFFIELDTYPES 50

// declare table for fields
struct _field {	
	char fieldName[MAXLENOFFIELDNAMES];
	char fieldType[MAXLENOFFIELDTYPES];
	int fieldLength;
};

// declare table variables
struct _table {
	char *tableFileName;
	char *schemaName;
	char *fName;
	int reclen;
	int fieldcount;
	struct _field fields[MAXFIELDS];
	struct _table *next;
};

// declare functions
typedef enum{false, true} bool;
bool processCommand(char *buffer, FILE *fileStream);
char *trimwhitespace(char *str);
bool AndClauseCheck(struct _table *table, char *record, char *secFieldName, char *secFieldValue);

// this is function sets a list of tables
void tableList(struct _table *list, struct _table *head, char *name){	
	char *token;
	token = strtok(name, ", \n");
	list->fName = calloc(1, strlen(token) + 1);
	strcpy(list->fName, token);
	token = strtok(NULL, ", \n");
	while(token != NULL){
		list->next = calloc(1,sizeof(struct _table));
		list->next->fName = calloc(1, strlen(token) + 1);
        	strcpy(list->next->fName, token);
        	list = list->next;
        	token = strtok(NULL, ", \n");   
    	}
	list->next = NULL;
}

// this function generate .bin and .schema file name for tables
void generateTableName(struct _table *table, char *token) {
	table->fName = (char*)calloc(1, strlen(token) + 1);
	table->tableFileName = (char*)calloc(1, strlen(token) + 5);
	table->schemaName = (char*)calloc(1, strlen(token) + 8);
	strncpy(table->fName,token,strlen(token));
	strncpy(table->tableFileName,token,strlen(token));
	strncpy(table->schemaName,token,strlen(token));
	strcat(table->tableFileName, ".bin");
	strcat(table->schemaName, ".schema");
    
}

// this function gets the last field in select fields
void getLastToken(char *selectStr, char *last){
	char *ret;
	char *token;
	ret = strrchr(selectStr, ',');
	if(ret == NULL){
		token = strtok(selectStr, "\n");
		strncpy(last, token, strlen(token)+1);
	}
	else{
		token = strtok(ret, ", ");
		strncpy(last, token, strlen(token)+1);
	}
}

// this function create bin file and schema file
bool createTable(struct _table *table, FILE *filestream){
	char str[MAXINPUTLENGTH];
	FILE *schema = fopen (table->schemaName, "w");
	FILE *file = fopen (table->tableFileName, "w");
	if(schema != NULL){
		while (fgets(str, sizeof(str), filestream)) {
            		//printf("===> %s", str);
            		fwrite(str , strlen(str), 1 , schema);
            		if(str[0] == 'E' ){
                		break;
          	  	}  
        	}
    	}
    	fclose(file);
    	fclose (schema);
}

// this function load data and insert records into the .bin file
bool loadDatabase(struct _table *table, char *token) {
	char *recordBuffer = calloc (1, table->reclen);
    	char *newToken;
    	int index = 0;
    	int i;
	
    	FILE *file = fopen(table->tableFileName, "ab");

    	//Fill up the buffer
    	newToken = strtok(token, ",\n");
	
    	for(i = 0; i < table->fieldcount; ++i){
        	if(strlen(token) <= table->fields[i].fieldLength){
            	strncpy(&recordBuffer[index], token, table->fields[i].fieldLength - 1);
        	}
        	index += table->fields[i].fieldLength;
        	token = strtok(NULL, ",\n");

        	//Stop reading input when next line is NULL
        	if(token == NULL){
            		break;
        	}
    	}
	  
    	fwrite(recordBuffer, table->reclen, 1, file);

    	free(recordBuffer);
    	fclose(file);
    	return 1;
}

// this function create a table by reading from .schema file and load into table
bool loadSchema(struct _table *table) {
	char str[MAXINPUTLENGTH];
    	int i = 0;
    	char *token;

    	//Initialize fieldcount and reclen so they won't added up in case calling loadSchema multiple time
    	table->fieldcount = 0;
    	table->reclen = 0;
    	FILE *schema;
    	schema = fopen(table->schemaName, "r");
    	if(schema != NULL){
        	while (fgets(str, sizeof(str), schema) && str[0] != '\n' ) {
            		token = strtok(str, " ");
            		if(strncmp(token, "END", 3) == 0){
                		break;
            		}
            
            		if(strncmp(token, "ADD", 3) == 0){
                		token = strtok(NULL, " ");
                		strncpy(table->fields[i].fieldName, token, sizeof(table->fields[i].fieldName));
				token = strtok(NULL, " ");
               			strncpy(table->fields[i].fieldType, token, sizeof(table->fields[i].fieldType));
                		token = strtok(NULL, " ");
                		int val = atoi(token);
                		table->fields[i].fieldLength = val;
                		table->reclen += val;
                		table->fieldcount += 1;
            		}
            		++i;
        	}
    	}
    	fclose(schema);

}

//Get record to populate the record
bool getRecord(int recnum, char *record, struct _table *table){
	FILE* file;
    	file = fopen(table->tableFileName, "rb");

    	fseek(file, recnum * table->reclen, SEEK_SET);
    	if(fread(record, table->reclen, 1, file) != 1){
        	printf("Fail!");
        	return 0;
    	}
    	fclose(file);
    	return 1;
}

//trims the white space off of the input
char *trimwhitespace(char *str){
	char *endStr;
    	// Trim leading space
    	while(isspace((unsigned char)*str))
        	str++;
    	if(*str == 0)
        	return str;
    	// Trim trailing space
    	endStr = str + strlen(str) - 1;
    	while(endStr > str && isspace((unsigned char)*endStr))
        	endStr--;

    	// Write new null terminator
    	*(endStr + 1) = 0;
    	return str;
}

// prints the information of selected fields
void showRecord(struct _field *fields, char *record, int fieldcount, char *token, char *last){
	int index = 0;
    	for(int i = 0; i < fieldcount; ++i){
        	if(strcmp(token, fields[i].fieldName) == 0){
            		if(strcmp(last, fields[i].fieldName) == 0){
                		trimwhitespace(&record[index]);
                		printf("%s", &record[index]);
            		}
            		else{
                		trimwhitespace(&record[index]);
                		printf("%s,", &record[index]);
            		}
        	}
        	index += fields[i].fieldLength;
    	}

}

// this function checks if the record retrieved passes the Where Clause test
bool whereClauseCheck(struct _table *table, char *record, char *fieldName, char *fieldValue, char *secFieldName, char *secFieldValue){
	int index = 0;
    	for(int i = 0; i < table->fieldcount; ++i){
		//case 1: there is only WHERE clause
        	if(secFieldName == NULL){
        		if(strncmp(fieldName, table->fields[i].fieldName, strlen(table->fields[i].fieldName)) == 0){
         			if(strstr(fieldValue, &record[index]) != 0){
            				return 1;
         			}
         			else{
            				return 0;
         			}
        		}
       		index += table->fields[i].fieldLength;
            
        	}
        
		//case 2: there are WHERE clause and AND clause
        	else{
        		if(strncmp(fieldName, table->fields[i].fieldName, strlen(table->fields[i].fieldName)) == 0){     
				if(strncmp(fieldValue, &record[index], strlen(&record[index])) == 0){
                			return AndClauseCheck(table, record, secFieldName, secFieldValue);
            			}
            			else{
                			return 0;
            			}
        		}
        		index += table->fields[i].fieldLength;
    		}
    	}
    
}

// additional check for AND Clause
bool AndClauseCheck(struct _table *table, char *record, char *secFieldName, char *secFieldValue){	
	int index = 0;
    	for(int i = 0; i < table->fieldcount; ++i){
        	if(strncmp(secFieldName, table->fields[i].fieldName, strlen(table->fields[i].fieldName)) == 0){      
            		if(strncmp(secFieldValue, &record[index], strlen(&record[index])) == 0){
                		return 1;
            		}
            		else{
                		return 0;
            		}
        	}
        	index += table->fields[i].fieldLength;
    	}
}

// this function checks if the record is in the desire range
bool rangeCheck(struct _table *table, char *record, char *rangeField, char *rangeValue1, char *rangeValue2){
	int index = 0;
	for(int i = 0; i < table->fieldcount; ++i){
        	if(strncmp(rangeField, table->fields[i].fieldName, strlen(table->fields[i].fieldName)) == 0){ 
            		if(strncmp(rangeValue1, &record[index], strlen(&record[index])) <= 0){
                		if(strncmp(rangeValue2, &record[index], strlen(&record[index])) >= 0){
					return 1;
				}
            		}
            		else{
                		return 0;
            		}
        	}
        	index += table->fields[i].fieldLength;
    	}
}

// this function opens file and gets number of record of a file 
int getFileSize(struct _table *table){	
	//Get the file size
	FILE *fp;
	int fileSize, recNum;
	fp = fopen(table->tableFileName, "r");
	fseek(fp, 0L, SEEK_END);
	fileSize = ftell(fp);
	rewind(fp);
	fclose(fp);
	recNum = fileSize / (table->reclen);
    	return recNum;
}

// this function gets the index of a field
int getIndex(struct _table *table, char *fieldName){
	FILE *tmpSchema;
    	tmpSchema = fopen(table->schemaName, "r");

    	int index = 0;
    	if(tmpSchema){
        	char *recStr = calloc(1, 200);
        	int i = 0;
        	while(1){
            		fgets(recStr, 200, tmpSchema);
            		if(strstr(recStr, fieldName)){
                		break;
            		}
            		index += table->fields[i].fieldLength;
            		i += 1;
        	}
        	free(recStr);
    	}
    	fclose(tmpSchema);
	return index;
}

// this function checks if the record is in the desire range
bool fieldCheck(char *record, int index, char *fieldValue1, char *fieldValue2){
	if(strncmp(fieldValue1, &record[index], strlen(&record[index])) <= 0){
              	if(strncmp(fieldValue2, &record[index], strlen(&record[index])) >= 0){
			return 1;
		}
        }
        else{
                return 0;
        }
}

void joinTables(struct _table *newTable, struct _table *table, struct _table *tmp, struct _table *tmpTable, struct _table *head, char *temp, char *field2, char *field1, int tableNum){
	// declare variables
	char *token;
    	tmp = head;
    	FILE *tmpSchema;
    	FILE *schema1;
    	FILE *schema2;
   	
    	//create and load first table
    	generateTableName(table, tmp->fName);
    	loadSchema(table);
    	int recNum1 = getFileSize(table);
    	
    	//create and load second table
    	generateTableName(tmpTable, tmp->next->fName);
	loadSchema(tmpTable);
    	
	int recNum2 = getFileSize(tmpTable);
    	
    	//create new table that joins two tables
    	char *newFileName = calloc(1, 8);
    	sprintf(newFileName, "table_%d", tableNum);
    	generateTableName(newTable, newFileName);
    	
    	//open new table to write and close it in case it overwrite 
    	FILE *schema = fopen (newTable->schemaName, "w");
    	fclose(schema);
    
    	char c1,c2;
    	char buff[100];
    
    	tmpSchema = fopen(newTable->schemaName, "a");
    	schema1 = fopen(table->schemaName, "r");
   	schema2 = fopen(tmpTable->schemaName, "r");
    
    	//copy content of first table schema file to new table schema file 
    	if(schema1){
        	while (fgets(buff, sizeof(buff), schema1)) {
            		if(buff[0] == 'E' ){
                		break;
            		}
            		fwrite(buff , strlen(buff), 1 , tmpSchema);
        	}
    	}
    	fclose(schema1);
    
    	//copy content of second table schema file to new table schema file 
    	if(schema2){
        	while (fgets(buff, sizeof(buff), schema2)) {
            		fwrite(buff , strlen(buff), 1 , tmpSchema);
            		if(buff[0] == 'E' ){
                		break;
            		}
        	}   
    	}
    	fclose(schema2);
    	fclose(tmpSchema);

    	loadSchema(newTable);
    	char *record = calloc(1, newTable->reclen);
    	char *tmpBuffer1 = calloc (1, table->reclen);
    	char *tmpBuffer2 = calloc (1, tmpTable->reclen);
    
    	// get index for field1
    	schema1 = fopen(table->schemaName, "r");
    	int index1 = 0;
    	if(schema1){
        	char *string1 = calloc(1, 50);
        	int i = 0;
        	while(1){
            		fgets(string1, 50, schema1);
            		token = strtok(string1, " \n");
            		token = strtok(NULL, " \n");
            		if(strcmp(token, field1) == 0){
                		token = strtok(NULL, " \n");
                		token = strtok(NULL, " \n");
                		break;
            		}
            		token = strtok(string1, " \n");
            		token = strtok(NULL, " \n");
            		index1 += table->fields[i].fieldLength;
            		i += 1;
        	}
        	free(string1);
    	}
    	fclose(schema1);
    
    	// get index for field2
    	schema2 = fopen(tmpTable->schemaName, "r");
    	int index2 = 0;
   	if(schema2){
        	char *string2 = calloc(1, 50);
        	int i = 0;
        	fgets(string2, 50, schema2);
        	token = strtok(string2, " \n");
        	token = strtok(NULL, " \n");
        	while(1){
            		if(strcmp(token, field2) == 0){
                		token = strtok(NULL, " \n");
                		token = strtok(NULL, " \n");
                		break;
            		}

            		fgets(string2, 50, schema2);
            		token = strtok(string2, " \n");
            		token = strtok(NULL, " \n");
            		index2 += tmpTable->fields[i].fieldLength;
            		i += 1;
        	}
        	free(string2);
    	}
    	fclose(schema2);
 					
    	FILE *file1;
    	FILE *file2;
    
    	// open new table file to write in case overwrite
    	FILE *file = fopen (newTable->tableFileName, "w");
    	fclose(file);
    
    	// write the data from both tables to new table
    	char *recordBuff = calloc (1, table-> reclen + tmpTable->reclen + 1);
    	for(int i = 0; i < recNum1; i++){
        	file1 = fopen(table->tableFileName, "rb");
        	if(file1){
            		fseek(file1, i * table->reclen, SEEK_SET);
            		fread(tmpBuffer1, table->reclen, 1, file1);
        	}
        	fclose(file1);
        
        	for(int i = 0; i < recNum2; i++){
            		file2 = fopen(tmpTable->tableFileName, "rb");
            		if(file2){
                		fseek(file2, i * tmpTable->reclen, SEEK_SET);
                		fread(tmpBuffer2, tmpTable->reclen, 1, file2);
            		}
            		fclose(file2);
            	
			if(strcmp(&tmpBuffer2[index2], &tmpBuffer1[index1]) == 0){
                		memcpy(recordBuff, tmpBuffer1, table-> reclen);
                		memcpy(recordBuff + table-> reclen, tmpBuffer2, tmpTable->reclen);
                		FILE *newFile = fopen(newTable->tableFileName, "ab");
            
	    			if(newFile){
                    			fwrite(recordBuff, table->reclen + tmpTable->reclen, 1, newFile);
                		}
                		fclose(newFile);
            		}
        	}
    	}
    	free(newFileName);
    	free(tmpBuffer1);
    	free(tmpBuffer2);
    	free(recordBuff);
    	free(record);

}

// this function create a new table after join all nescessary the tables
int generateJoinedTable(char *token, char *fromStr, char *tmpStr, char *status, char *buffer, struct _table *table, struct _table *newTable, struct _table *tmpTable, struct _table *list, struct _table *head, struct _table *tmp, char *field1, char *field2, char *fieldName, char *fieldValue, char *rangeField, char *rangeValue1, char *rangeValue2, char *newStr, char *tableNameStr, char *temp, int tableNum, int count, int sortFlag){
			
                	// joins table using linked list
                	while(tmp->next != NULL){
                    		while(1){
                        		// breakout the loop if command is "END", return the sortFlag for sort check
                        		if(strncmp(token, "END", 3) == 0){
						return sortFlag;
                        		}
					// if command is "ORDER BY", set sortFlag to 1
					else if(strncmp(token, "ORDER", 5) == 0){
						sortFlag = 1;
						status = fgets(buffer, MAXINPUTLENGTH-1, stdin);
                            		//	printf("===> %s", status);
                            			token = strtok(status, " \n");
                            			continue;
					
					}
                        		else{
                            			//if command is 'WHERE' or 'AND', join tables 
                            			if(strncmp(token, "WHERE", 5) == 0 || strncmp(token, "AND", 3) == 0){
                                			token = strtok(NULL, "\n");
                                			//if the command contains variable with " ", store fieldName and fieldValue information for show record
                                			if(strstr(token, "\"")){
								token = strtok(token, " ");
                                    				strncpy(fieldName, token, strlen(token));
                                    				token = strtok(NULL, "= \n");
                                    				strncpy(fieldValue, token, strlen(token));
                                    				count += 1;
                                			}
							// if command contains ">=", store range value of the field
							else if(strstr(token, ">=")){
								token = strtok(token, " <=>=");
								strncpy(rangeField,token, strlen(token));
								token = strtok(NULL, " <=>='\n");
								strncpy(rangeValue1,token, strlen(token));
								count += 1;
							}
							// if command contains ">=", store range value of the field
							else if(strstr(token, "<=")){
								token = strtok(token, " <=>=");	
								token = strtok(NULL, " <=>='\n");
								strncpy(rangeValue2,token, strlen(token));
								count += 1;
							}
							// otherwise join the tables
                                			else{
								token = strtok(token, " ");
                                    				strncpy(field1, token, strlen(token)+1);
                                    				token = strtok(NULL, "= \n");
                                    				strncpy(field2, token, strlen(token)+1);
								// function that perform join table
                                    				joinTables(newTable, table, tmp, tmpTable, head, temp, field2, field1, tableNum);
                                    				//change node pointers to see if there is another table to join
                                    				tableNum += 1;
                                    				head = newTable;
                                    				head->next = tmp->next->next;
                                    				tmp = head; 
                                    				count += 1;
                                			}
						}
						// continue to next command 
                            			status = fgets(buffer, MAXINPUTLENGTH-1, stdin);
                            		//	printf("===> %s", status);
                            			token = strtok(status, " \n");
                            			continue;
                        		}
                    		}
                	}
}

// this function implement different logics depending on the input commands
bool processCommand(char *buffer, FILE *fileStream){
	// declare variables
	struct _table *table = calloc(1, sizeof(struct _table));
    	char *token;
    	token = strtok(buffer, " ");
		
   	// Create table command
    	if(strncmp(token, "CREATE", 6) == 0){
		int sortFlag = 0;
        	token = strtok(NULL, " ");
        	if(strncmp(token, "TABLE", 5) == 0){
           		token = strtok(NULL, "\n");
            		generateTableName(table, token);
            		createTable(table, fileStream);
            		free(table->fName);
            		free(table->tableFileName);
            		free(table->schemaName);
            		return 0;
        	}
        
        // Create index command
        if(strncmp(token, "INDEX", 5) == 0){
	    	token = strtok(NULL, " ");
            
            	//store commands for temp file
            	char *cmd = calloc(1, strlen(token) + 14);
            	char *insertCmd = calloc(1, strlen(token) + 13);
            	sprintf(cmd, "CREATE TABLE %s\n", token);
            	sprintf(insertCmd, "INSERT INTO %s", token);
            
            	struct _table *indexTable = calloc(1, sizeof(struct _table));
            	generateTableName(indexTable, token);
            	token = strtok(NULL, " ");
    		
		// store select fields for show record
		if(strncmp(token, "USING", 5) == 0){
            		token = strtok(NULL, "\n");
                	char *tmpBuff = calloc(1, strlen(token)+1);
                	char *tempBuffer = calloc(1, strlen(token)+1);
                	strncpy(tmpBuff, token, strlen(token));
                	strncpy(tempBuffer, token, strlen(token));
                	char *status = fgets(buffer, MAXINPUTLENGTH-1, stdin);
        	//	printf("===> %s", status);
        		token = strtok(status, ", ");
			 
			// store tables for join
                	if(strncmp(token, "FROM", 4) == 0){
                		token = strtok(NULL, "\n");
				char *fromStr = calloc(1, strlen(token)+1);
            			strncpy(fromStr, token, strlen(token));
            			char *tmpStr = calloc(1, strlen(fromStr)+1);
            			strcpy(tmpStr, fromStr);

				// initialize tables
				struct _table *newTable = calloc(1, sizeof(struct _table));
                		struct _table *tmpTable = calloc(1, sizeof(struct _table));
                		struct _table *list = calloc(1, sizeof(struct _table));
    				struct _table *head = list;
                		struct _table *tmp = head;
				
				// initialize neccessary fields
               			char *field1 = calloc(1, strlen(token)+1);
                		char *field2 = calloc(1,strlen(token)+1);
                		char *fieldName = calloc(1, strlen(token)+1);
                		char *fieldValue = calloc(1, strlen(token)+1);
                		char *rangeField = calloc(1, strlen(token)+1);
				char *rangeValue1 = calloc(1, strlen(token)+1);
				char *rangeValue2 = calloc(1, strlen(token)+1);

      	          		memcpy(fromStr, tmpStr, strlen(tmpStr)+1);
                
                		//create list of tables
                		tableList(list, head, fromStr);
                
                		char newStr[100];
                		char *tableNameStr = calloc(1, 100);
                		char *temp= calloc(9999, sizeof(char));

                		status = fgets(buffer, MAXINPUTLENGTH-1, stdin); 
                	//	printf("===> %s", status);
                		token = strtok(status, " ");
				
				// initialize number of table and count to 0
                		int tableNum = 0;
                		int count = 0;
					
				// generate the joined table
				sortFlag = generateJoinedTable(token, fromStr, tmpStr, status, buffer, table, newTable, tmpTable, list, head, tmp, field1, field2, fieldName, fieldValue, rangeField, rangeValue1, rangeValue2, newStr, tableNameStr, temp, tableNum, count, sortFlag);
				
				// declare variables
                    		int recordNum = getFileSize(newTable);			
                    		int index;
                    		char *recBuff = calloc(1, newTable->reclen);
				char *temp_Buffer = calloc(1, strlen(tmpBuff)+1);
                    		int i = 0;
                    		int bufflen;
				
				// create a sort file
				FILE *sortfile = fopen("sortme.sortme", "w");
				// open the joined table for read
				FILE *fp = fopen(newTable->tableFileName, "rb");
				
				// initialize fields
				char *secFieldName;
				secFieldName = 0;
				char *secFieldValue;
				secFieldValue = 0;

				// reads desire record and write into sort file
                    		for(int i = 0; i < recordNum; i++){
					if(fp){
						fseek(fp, i * newTable->reclen, SEEK_SET);
                				fread(recBuff, newTable->reclen, 1, fp);
						memcpy(temp_Buffer, tmpBuff, strlen(tmpBuff));
						token = strtok(temp_Buffer, ",");
						if(rangeCheck(newTable, recBuff, rangeField, rangeValue1, rangeValue2)){
							if(whereClauseCheck(newTable, recBuff, fieldName, fieldValue, secFieldName, secFieldValue)){		
								while(token != NULL){
                        						index = getIndex(newTable, token);
                        						token = strtok(NULL, ", \n");
									if(token == NULL){
										fprintf(sortfile, "%s\n", &recBuff[index]);
									}
									else{
										fprintf(sortfile, "%s,", &recBuff[index]);
									}
								}	
							}
						}
					}
                    		}
                    		free(recBuff);
                    		free(tmpBuff);
                    		fclose(fp);
                    		fclose(sortfile);
				
				// check if we need to sort the file
				char *sort_file;
                    		if(sortFlag == 1){
					system("sort <sortme.sortme>sortme.sorted");
					sort_file = "sortme.sorted";
                    		}
				else{
					sort_file = "sortme.sortme";	
				}

				// open temp file to store commands
                    		FILE *tempFile;
            			tempFile = fopen("temp", "w");
            			fwrite(cmd, strlen(cmd), 1, tempFile);
            			free(cmd);

				// read from joined table and load desire commands into temp file
                    		char schemaStr[100];
                    		token = strtok(tempBuffer, ", ");
                    		FILE *schemaFile = fopen(newTable->schemaName, "r");
                    		while (fgets(schemaStr, sizeof(schemaStr), schemaFile)) {
                        		if(token == NULL){
                            			fputs("END\n", tempFile);
                            			break;
                        		}
            				if(schemaStr[0] == 'E' ){
						rewind(schemaFile);
                				
            				}
                        		if(strstr(schemaStr, token)){
                            			fwrite(schemaStr, strlen(schemaStr), 1 , tempFile);
                            			token = strtok(NULL, ", \n");
                        		}
        			}
                    		free(tempBuffer);
				fclose(schemaFile);
                    		
				// read from sort file and load desire commands into temp file
                    		sortfile = fopen(sort_file, "r");
				while(1){
                        		char *tmpStr = malloc(300);
					char *sbuf = malloc(100);
                       			if(fgets(tmpStr, 300, sortfile) == NULL){
                            			break;
                        		}
					else{		
						sprintf(sbuf, "%s %s", insertCmd, tmpStr);
						fwrite(sbuf, strlen(sbuf), 1, tempFile);
					}
                        		free(tmpStr);
                        		free(sbuf);	
				}
                    		free(insertCmd);
                    		fclose(sortfile);
            			fclose(tempFile);
                    		
                   		// read from temp file to create table and insert data
                    		tempFile = fopen("temp", "r");
    				memset(buffer, 0, MAXINPUTLENGTH);
 				status = fgets(buffer, MAXINPUTLENGTH-1, tempFile);
                    		token = strtok(status, " ");
					
				// create tables and load datas
                    		if(strncmp(token, "CREATE", 6) == 0){
        				token = strtok(NULL, " ");
        				if(strncmp(token, "TABLE", 5) == 0){
            					token = strtok(NULL, "\n");
            					generateTableName(table, token);
						
                    				char str[MAXINPUTLENGTH];
    						FILE *schema = fopen (table->schemaName, "w");
    						FILE *file = fopen (table->tableFileName, "w");
						if(schema != NULL){
        						while (fgets(str, sizeof(str), tempFile)) {
            							fwrite(str , strlen(str), 1 , schema);
            							if(str[0] == 'E' ){
                							break;
            							}
        						}
    						}
    						fclose(file);
    						fclose (schema);
            					free(table->fName);
            					free(table->tableFileName);
            					free(table->schemaName);
						
						// continue to the next command
                            			status = fgets(buffer, MAXINPUTLENGTH-1, tempFile);
                            			while (status != NULL) {
        						trimwhitespace(buffer);
        						if (strlen(buffer) < 5)
            							break; // not a real command, CR/LF, extra line, etc.
        						processCommand(buffer, tempFile);
        						status = fgets(buffer, MAXINPUTLENGTH-1, tempFile);
    						}
					}
                    		}
                    		return 0;
				}
			}     
		}
	}
   
    	// load the data to table
    	if(strncmp(token, "INSERT", 6) == 0){
        	token = strtok(NULL, " ");
        	if(strncmp(token, "INTO", 4) == 0){
            		token = strtok(NULL, " \n");
			generateTableName(table, token);
            		token = strtok(NULL, "\n");
            		loadSchema(table);
            		loadDatabase(table, token);
            		free(table->fName);
            		free(table->tableFileName);
            		free(table->schemaName);
        	}
    	}
   	
	// store select fields
    	if(strncmp(token, "SELECT", 6) == 0){
		token = strtok(NULL, "\n");
        	char *selectStr= calloc(1, strlen(token)+1);

        	strncpy(selectStr, token, strlen(token));

        	//countinue to next input line 
        	char *status = fgets(buffer, MAXINPUTLENGTH-1, stdin);
        //	printf("===> %s", status);
        	token = strtok(status, ", ");
		
        	//From statement input after Select statement
        	if(strncmp(token, "FROM", 4) == 0){
            	char *newToken;
            
            	token = strtok(NULL, "\n");
            	char *fromStr = calloc(1, strlen(token)+1);
            	strncpy(fromStr, token, strlen(token));
            	char *tmpStr = calloc(1, strlen(fromStr)+1);
            	strcpy(tmpStr, fromStr);
            
            	newToken = strtok(fromStr, ",\n");

            	//case that select from only one table
            	if(strncmp(newToken, tmpStr, strlen(tmpStr)) == 0){
				
                	generateTableName(table, token);
                	loadSchema(table);

                	//Get the file size
                	int recNum = getFileSize(table);

                	//continue to next input line
                	status = fgets(buffer, MAXINPUTLENGTH-1, stdin); 
                	//printf("===> %s", status);
                
        		
                	//if there is no where clause
                	if(strncmp(status, "END", 3) == 0){
			//	printf("enter end\n");
                    		char *record = calloc(1,table->reclen);

                    		//get the last token of select fields
                    		char *last = calloc(1,10);
                    		getLastToken(selectStr, last);

                    		//store select fields 
                    		char *newStr = calloc(1, 100);
                    		strncpy(newStr, selectStr, strlen(selectStr));
                  		
				printf("OUTPUT4:\n");
                    		//prints the desire output
                    		for(int i = 0; i < recNum; i++){
                        		//get stored fields
                        		memcpy(selectStr, newStr, strlen(newStr));
                        		token = strtok(selectStr, ", ");
                        		if(getRecord(i, record, table)){
                            			while(1){
                                			if(token == NULL){
                                    				break;
                                			}
                                			showRecord(table->fields, record, table->fieldcount, token, last);
                                			token = strtok(NULL, ", \n");
                            			}
                            			printf("\n");
					}
                    		}
                    		free(last);
                    		free(newStr);
                    		free(record);
                	}
                
                	//case that there are WHERE clause and And Clause 
                	else{
                    		token = strtok(status, " ");
                    		if(strncmp(token, "WHERE", 5) == 0){
                        		token = strtok(NULL, " ");
					char *secFieldName;
					char *secFieldValue;
                        		char *fieldName = calloc(1,strlen(token));
                        		strncpy(fieldName, token, strlen(token));
                        		token = strtok(NULL, ">= \'");
                        		char *fieldValue = calloc(1,strlen(token));
                        		strncpy(fieldValue, token, strlen(token));

                        		char *record = calloc(1,table->reclen);
                        		status = fgets(buffer, MAXINPUTLENGTH-1, stdin); 
                        	//	printf("===> %s", status);
                        		token = strtok(status, " ");
                        
                        		if(strncmp(token, "AND", 3) == 0){
                            			token = strtok(NULL, " ");
                            			secFieldName = calloc(1,strlen(token));
                            			strncpy(secFieldName, token, strlen(token));
                            			token = strtok(NULL, "<= \'");
                            			secFieldValue = calloc(1,strlen(token));
                           			strncpy(secFieldValue, token, strlen(token));
                            			
						status = fgets(buffer, MAXINPUTLENGTH-1, stdin); 
                            		//	printf("===> %s", status);
                           			token = strtok(status, " ");
 
                        		}

                        		// perform binary search if the command is "END"
                        		if(strncmp(token, "END", 3) == 0){
						
						int index;
                            			index = getIndex(table, fieldName);

                            			int recNum = getFileSize(table);
                            			char array[recNum][20];
						// get record
                            			for(int i = 0; i < recNum; i++){
                            				if(getRecord(i, record, table)){
                                    				strcpy(array[i], &record[index]); 
                                			}
                            			}
						free(record);
                            			char *last = calloc(1, 10);
                            			getLastToken(selectStr, last);

                            			//reserve select fields
                            			char *newStr = calloc(1, 100);
                            			strncpy(newStr, selectStr, strlen(selectStr));

						// perform binary search for desire fields
						int left = 0;
						int right = recNum - 1;
						int mid;
						bool found = false;
						while(left <= right && !found)
						{
							mid = (left + right) / 2;

							if(strcmp(array[mid], fieldValue) == 0 ){
								found = true;
								static int outputNum = 0;
								outputNum++;
								printf("OUTPUT%d:\n", outputNum);
								// make sure starting from the right record
								while(strncmp(array[mid-1], fieldValue, strlen(fieldValue)) == 0){
									mid = mid - 1;
								}
								// searching right records all the way down 
								while(mid < recNum){
									char *rec_buf = calloc(1,table->reclen);
									memcpy(selectStr, newStr, strlen(newStr)+1);
                            						token = strtok(selectStr, ", ");
            								if(getRecord(mid, rec_buf, table)){
										// stop when out of range 
										if(&rec_buf[index] > secFieldValue){
											break;
										}
										// check if the record is in desire range
										if(fieldCheck(record, index, fieldValue, secFieldValue)){
											while(1){
                                    								if(token == NULL){
                                            								break;
                                        							}
                                        							showRecord(table->fields, rec_buf, table->fieldcount, token, last);
                                        							token = strtok(NULL, ", \n");
                                    							}
                                        						printf("\n");
										}
									}
									mid += 1;
									free(rec_buf);
								}
            						
        						}
							else if (strcmp(array[mid], fieldValue) > 0){
								right = mid - 1;
                                    				memcpy(selectStr, newStr, strlen(newStr)+1);
                            					token = strtok(selectStr, ", ");
        						}
							
							else{
								left = mid + 1;
                                    				memcpy(selectStr, newStr, strlen(newStr)+1);
                            					token = strtok(selectStr, ", ");

        						}
						}
						
                            			free(last);
                            			free(newStr);
                        		}
                        
                        		free(fieldName);
                        		free(fieldValue);
                    		}
                	}
            	}
            
            	//case that select from two tables and needed to be join as one table
            	else{
                	struct _table *newTable = calloc(1, sizeof(struct _table));
                	struct _table *tmpTable = calloc(1, sizeof(struct _table));
                	struct _table *list = calloc(1, sizeof(struct _table));
    			struct _table *head = list;
                	struct _table *tmp = head;
			
               		char *field1 = calloc(1, strlen(token)+1);
                	char *field2 = calloc(1,strlen(token)+1);
                	char *fieldName = calloc(1, strlen(token)+1);
                	char *fieldValue = calloc(1, strlen(token)+1);
                	char *rangeField = calloc(1, strlen(token)+1);
			char *rangeValue1 = calloc(1, strlen(token)+1);
			char *rangeValue2 = calloc(1, strlen(token)+1);

                	memcpy(fromStr, tmpStr, strlen(tmpStr)+1);
                
                	//create list of tables
                	tableList(list, head, fromStr);
                
                	char newStr[100];
                	char *tableNameStr = calloc(1, 100);
                	char *temp= calloc(9999, sizeof(char));

                	status = fgets(buffer, MAXINPUTLENGTH-1, stdin); 
                	printf("===> %s", status);
                	token = strtok(status, " ");

                	int tableNum = 0;
                	int count = 0;
			int sortFlag;			

			// generate the joined table
			generateJoinedTable(token, fromStr, tmpStr, status, buffer, table, newTable, tmpTable, list, head, tmp, field1, field2, fieldName, fieldValue, rangeField, rangeValue1, rangeValue2, newStr, tableNameStr, temp, tableNum, count, sortFlag);
                	char *record = calloc(1, newTable->reclen);
                
                	//if the command is 'END', prints desire output base on the condition
                	if(strncmp(token, "END", 3) == 0){
				
                    		//case that only joined two tables once 
                    		if(count == 1){
				
                        		//Get the file size
                        		int recNum = getFileSize(newTable);
                        
                        		//get the last token of select fields
                        		char *last = calloc(1,20);
                        		getLastToken(selectStr, last);
                        
                        		//stored select fields
                        		char *newStr = calloc(1,strlen(selectStr));
                        		strncpy(newStr, selectStr, strlen(selectStr));
                        
                        		for(int i = 0; i < recNum; i++){
                            			//get stored fields
                            			memcpy(selectStr, newStr, strlen(newStr));
                            			token = strtok(selectStr, ", ");
                            			if(getRecord(i, record, newTable)){
                                			while(1){
                                    				if(token == NULL){
                                        				break;
                                    				}
                                    				showRecord(newTable->fields, record, newTable->fieldcount, token, last);
                                    				token = strtok(NULL, ", \n");
                                			}
                                			printf("\n");
                            			}
                        		}
					free(last);
                        		free(newStr);
                        		return 0;
                    		}
                    		
				//case that there are multiple tables joined  
                    		else{
					char *record = calloc(1, newTable->reclen);
                        		char *last = calloc(1,20);
                        		getLastToken(selectStr, last);

                        		//store select fields 
                        		char *newStr = calloc(1,strlen(selectStr)+1);
                        		strncpy(newStr, selectStr, strlen(selectStr));
					
                        		char *secFieldName;
                        		secFieldName = 0;
                        		char *secFieldValue;
                        		secFieldValue = 0;
                        		int recNum = getFileSize(newTable);
					
					printf("\nOUTPUT:\n");
                        		for(int i = 0; i < recNum; i++){
                            			//get stored fields
                            			memcpy(selectStr, newStr, strlen(newStr));
                            			token = strtok(selectStr, ", ");
                            			if(getRecord(i, record, newTable)){
							if(rangeCheck(newTable, record, rangeField, rangeValue1, rangeValue2)){
                                				if(whereClauseCheck(newTable, record, fieldName, fieldValue, secFieldName, secFieldValue)){
                                    					while(1){
										if(token == NULL){
                                            						break;
                                        					}
                                        					showRecord(newTable->fields, record, newTable->fieldcount, token, last);
                                        					token = strtok(NULL, ", \n");
                                    					}
                                    					printf("\n");
                                				}
							}
                                			else{
                                    				continue;
                                			}
                            			}
					}
                        		printf("\n");
					free(record);
					free(last);
					free(newStr);
					free(newTable);
					free(tmpTable);
					free(list);
					free(field1);
					free(field2);
					free(fieldName);
					free(fieldValue);
					free(rangeField);
					free(rangeValue1);
					free(rangeValue2);
					free(temp);
					free(tableNameStr);
                        		return 0;
                    		}
                	}

		}
            	free(fromStr);
        	free(tmpStr);
        }
        free(selectStr);
    	}
}


int main() {
	static char buffer[MAXINPUTLENGTH];
    	memset(buffer, 0, MAXINPUTLENGTH);
 	char *status = fgets(buffer, MAXINPUTLENGTH-1, stdin);
	while (status != NULL) {
        	trimwhitespace(buffer);
        	if (strlen(buffer) < 5)
            		break; // not a real command, CR/LF, extra line, etc.
        	//printf("===> %s\n", buffer);
        	processCommand(buffer, stdin);
        	status = fgets(buffer, MAXINPUTLENGTH-1, stdin);
    	}
 	return 0;
}
