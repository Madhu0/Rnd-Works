//#include"WinServer.cpp"

#define BITVECTOR 8184
#define USERS 32768
#define FILESMETA 1048576
#define USERCOUNTS 32736
#define BLOCKSIZE 4096
#define USERSIZE 32
#define FILEMETASIZE 32



struct filemeta{
	char name[20];
	int size;
	int nextblock;
	int nextfile;
};


int TestBit(int values, int pos)
{
	int flag = 1;
	flag = flag << pos;
	if (flag&values)
		return 1;
	else
		return 0;
}


void ClearBit(int offset, FILE *fp){
	int flag = 1, index, pos, value;
	pos = (offset - FILESMETA) / BLOCKSIZE;
	index = pos / 32;
	fseek(fp, index * 4, SEEK_SET);
	fread(&value, sizeof(int), 1, fp);
	flag = flag << pos;
	flag = ~flag;
	value = value&flag;
	fseek(fp, index*4, SEEK_SET);
	fwrite(&value, sizeof(int), 1, fp);
	fflush(fp);
}


void SetBit(int offset, FILE *fp){
	int flag = 1, index, pos, value;
	pos = (offset - FILESMETA) / BLOCKSIZE;
	index = pos / 32;
	fseek(fp, index * 4, SEEK_SET);
	fread(&value, sizeof(int), 1, fp);
	flag = flag << pos;
	value = value | flag;
	fseek(fp, index * 4 , SEEK_SET);
	fwrite(&value, sizeof(int), 1, fp);
}


int find_empty_bit(FILE *fp){
	int i, j, num, flag = 0, offset;
	fseek(fp, 0, SEEK_SET);
	for (i = 0; i < BITVECTOR; i++){
		fseek(fp, 4 * i , SEEK_SET);
		fread(&num, sizeof(int), 1, fp);
		if (num != INT_MAX){
			for (j = 0; j < 32; j++){
				if (!TestBit(num, j)){
					offset = FILESMETA + (i * 32 + j)*BLOCKSIZE;
					SetBit(offset, fp);
					return offset;
				}
			}
		}
	}
	return -1;
}

void deallocate(int offset, FILE *fp){
	struct filemeta fm;
	fseek(fp, offset, SEEK_SET);
	fread(&fm, 32, 1, fp);
	ClearBit(offset, fp);
	offset = fm.nextblock;
	while (offset != 0){
		fseek(fp, offset, SEEK_SET);
		fread(&fm, 32, 1, fp);
		ClearBit(offset, fp);
		offset = fm.nextblock;
	}
}


void delete_file(int ch, FILE *fp,int *csock){
	struct filemeta fm, prevfm;
	int offset, prevoffset, i=0;
	offset = current_logged_user.fileoffset;
	prevoffset = offset;
	while (ch != i){
		i++;
		fseek(fp, offset, SEEK_SET);
		fread(&fm, 32, 1, fp);
		if (ch != i){
			prevoffset = offset;
			offset = fm.nextfile;
		}
	}
	if (ch == 0){
		fseek(fp, offset, SEEK_SET);
		fread(&fm, 32, 1, fp);
		current_logged_user.fileoffset = fm.nextfile;
		fseek(fp, offset_clu, SEEK_SET);
		fwrite(&current_logged_user, 32, 1, fp);
		fflush(fp);
		deallocate(offset, fp);
	}
	else{
		fseek(fp, prevoffset, SEEK_SET);
		fread(&prevfm, 32, 1, fp);
		fseek(fp, offset, SEEK_SET);
		fread(&fm, 32, 1, fp);
		prevfm.nextfile = fm.nextfile;
		fseek(fp, prevoffset, SEEK_SET);
		fwrite(&prevfm, 32, 1, fp);
		deallocate(offset, fp);
	}
	replyto_client("success", csock);
}


void download_file(int ch,FILE *fp,int *csock){
	int i=0,offset,block,length;
	char buff[4064],temp[10];
	buff[0] = '\0';
	char *reply;
	struct user u;
	struct filemeta fm;
	//reply = receive_data(csock);
	//ch = reply[0] - '0';
	offset = current_logged_user.fileoffset;
	while (i != ch){
		fseek(fp, offset, SEEK_SET);
		fread(&fm, 32, 1, fp);
		i++;
		if (i!=ch)
			offset = fm.nextfile;
	}
	fseek(fp, -32, SEEK_CUR);
	strcat(buff, fm.name);
	strcat(buff, "\n");
	itoa(fm.size, temp, 10);
	strcat(buff, temp);
	replyto_client(buff, csock);
	receive_data(csock);
	block = fm.size;
	while (block > 0){
		memset(buff, 0, 4064);
		block = block - 4064;
		printf("\n\n%d\n\n", block);
		fseek(fp, offset, SEEK_SET);
		fread(&fm, 32, 1, fp);
		length=fread(buff, 1, 4064,fp);
		replyto_client(buff, csock);
		receive_data(csock);
		if (block < 0){
			itoa(4064+block, temp, 10);
			replyto_client(temp, csock);
			receive_data(csock);
			//buff[length] = EOF;
		}
		offset = fm.nextblock;
	}
	//replyto_client("Hello", csock);
}


void display_files(FILE *fp, int *csock){
	char msg[] = "0";
	char filenames[4064], *reply,*temp;
	struct filemeta fm;
	while (1){
		filenames[0] = '\0';
		if (current_logged_user.fileoffset == 0){
			replyto_client(msg, csock);
			receive_data(csock);
		}
		else{
			fseek(fp, current_logged_user.fileoffset, SEEK_SET);
			fread(&fm, sizeof(struct filemeta), 1, fp);
			strcat(filenames, "\n");
			strcat(filenames, fm.name);
			while (fm.nextfile != 0){
				fseek(fp, fm.nextfile, SEEK_SET);
				fread(&fm, sizeof(struct filemeta), 1, fp);
				strcat(filenames, "\n");
				strcat(filenames, fm.name);
			}
			replyto_client(filenames, csock);
			receive_data(csock);
		}
		reply = receive_data(csock);
		replyto_client("ok", csock);
		if (reply[0] == 99)
			add_file(fp, csock);
		else if (reply[0] == 100)
			return;
		else{
			temp = receive_data(csock);
			replyto_client("ok", csock);
			if (temp[0] == 1)
				download_file(reply[0], fp, csock);
			else if (temp[0] == 2)
				delete_file(reply[0], fp,csock);
		}
	}
}


void add_file(FILE *fp, int *csock){
	char *reply,ch,*length;
	int i,j, offset, blocks, next_offset=0, prev_offset;
	struct filemeta fm, temp;
	memset(&fm, 0, 32);
	offset = find_empty_bit(fp);
	reply = receive_data(csock);
	replyto_client("ok", csock);
	for (i = 0; reply[i] != '\n'; i++){
		fm.name[i] = reply[i];
	}
	fm.name[i++] = '\0';
	for (; reply[i] != '\0'; i++)
		fm.size = fm.size * 10 + reply[i] - '0';
	i = 0;
	blocks = fm.size;
	fseek(fp, offset, SEEK_SET);
	if (current_logged_user.fileoffset == 0){
		while (blocks > 0){
			blocks = blocks - 4064;
			if (blocks==fm.size-4064)
				current_logged_user.fileoffset = offset;
			if (blocks>0){
				reply = receive_data(csock);
				replyto_client("ok", csock);
				next_offset = find_empty_bit(fp);
				fm.nextblock = next_offset;
				fseek(fp, offset, SEEK_SET);
				fwrite(&fm, 32, 1, fp);
				fflush(fp);
				fwrite(reply, 4064, 1, fp);
				fflush(fp);
			}
			else{
				length = receive_data(csock);
				replyto_client("ok", csock);
				reply = receive_data(csock);
				replyto_client("ok", csock);
				fm.nextblock = 0;
				fseek(fp, offset, SEEK_SET);
				fwrite(&fm, 32, 1, fp);
				fflush(fp);
				j = atoi(length);
				fwrite(reply, 1, j, fp);
				fflush(fp);
			}
			i++;
			offset = next_offset;
		}
	}
	else{
		fseek(fp, current_logged_user.fileoffset, SEEK_SET);
		fread(&temp, 32, 1, fp);
		offset = temp.nextfile;
		prev_offset = current_logged_user.fileoffset;
		while (offset != 0){
			prev_offset = offset;
			fseek(fp, offset, SEEK_SET);
			fread(&temp, 32, 1, fp);
			offset = temp.nextfile;
		}
		temp.nextfile = find_empty_bit(fp);
		offset = temp.nextfile;
		fseek(fp, prev_offset, SEEK_SET);
		fwrite(&temp, 32, 1, fp);
		while (blocks > 0){
			blocks = blocks - 4064;
			if (blocks>0){
				reply = receive_data(csock);
				replyto_client("received", csock);
				next_offset = find_empty_bit(fp);
				fm.nextblock = next_offset;
				fseek(fp, offset, SEEK_SET);
				fwrite(&fm, 32, 1, fp);
				fflush(fp);
				fwrite(reply, 4064, 1, fp);
				fflush(fp);
			}
			else{
				length = receive_data(csock);
				replyto_client("received", csock);
				reply = receive_data(csock);
				replyto_client("ok", csock);
				fm.nextblock = 0;
				fseek(fp, offset, SEEK_SET);
				fwrite(&fm, 32, 1, fp);
				fflush(fp);
				j = atoi(length);
				fwrite(reply, 1, j, fp);
				fflush(fp);
			}
			i++;
			offset = next_offset;
		}
	}
	fseek(fp, offset_clu, SEEK_SET);
	fwrite(&current_logged_user, 32, 1, fp);
	fflush(fp);
}
