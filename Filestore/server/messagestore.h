#define INIT 1677
#define START_MSG 12800
#define CATEGORY_START 7000
#define USER_START 5380
#define USER_STRUCT_SIZE 42
#define CAT_STRUCT_SIZE 30
#define MSG_SIZE 154
#define MAX_INT 2147483647

int categories = 0;

struct user_msg{
	char name[16];
	int no_of_cats;
	int offsets[5];
};

struct category{
	char name[20];
	int msg_offset;
	int no_of_msgs;
};

struct message{
	char name_of_owner[16];
	char contents[128];
	int next_msg;
	int reply_offset;
};

struct reply{
	char name_of_owner[16];
	char contents[128];
	int next_reply;
};


int TestBit_msg(int values, int pos)
{
	int flag = 1;
	flag = flag << pos;
	if (flag&values)
		return 1;
	else
		return 0;
}


void ClearBit_msg(int offset, FILE *fp){
	int flag = 1, index, pos, value;
	pos = (offset - START_MSG) / MSG_SIZE;
	index = pos / 32;
	fseek(fp, index * 4, SEEK_SET);
	fread(&value, sizeof(int), 1, fp);
	flag = flag << pos;
	flag = ~flag;
	value = value&flag;
	fseek(fp, offset, SEEK_SET);
	fwrite(&value, sizeof(int), 1, fp);
	fflush(fp);
}


void SetBit_msg(int offset, FILE *fp){
	int flag = 1, index, pos, value;
	pos = (offset - START_MSG) / MSG_SIZE;
	index = pos / 32;
	fseek(fp, index * 4 + 12, SEEK_SET);
	fread(&value, sizeof(int), 1, fp);
	flag = flag << pos;
	value = value | flag;
	fseek(fp, index * 4 + 12, SEEK_SET);
	fwrite(&value, sizeof(int), 1, fp);
}


int find_empty_bit_msg(FILE *fp){
	int i, j, num, flag = 0, offset;
	fseek(fp, 12, SEEK_SET);
	for (i = 0; i < INIT; i++){
		fseek(fp, 4 * i + 12, SEEK_SET);
		fread(&num, sizeof(int), 1, fp);
		if (num != MAX_INT){
			for (j = 0; j < 32; j++){
				if (!TestBit_msg(num, j)){
					offset = START_MSG + (i * 32 + j)*MSG_SIZE;
					SetBit_msg(offset, fp);
					return offset;
				}
			}
		}
	}
	return -1;
}


void delete_reply(FILE *fp, int msg_offset, int pos,int *csock){
	int temp_offset, rly_offset;
	struct reply r1, r2;
	struct message m;
	fseek(fp, msg_offset, SEEK_SET);
	fread(&m, sizeof(struct message), 1, fp);
	fseek(fp, m.reply_offset, SEEK_SET);
	fread(&r1, sizeof(struct reply), 1, fp);
	rly_offset = m.reply_offset;
	if (pos == 0){
		m.reply_offset = r1.next_reply;
		fseek(fp, msg_offset, SEEK_SET);
		fwrite(&m, sizeof(struct message), 1, fp);
		fflush(fp);
	}
	else{
		r2 = r1;
		while (pos != 0){
			if (pos == 2)
				rly_offset = r1.next_reply;
			fseek(fp, r1.next_reply, SEEK_SET);
			fread(&r1, sizeof(struct reply), 1, fp);
			if (pos == 2){
				r2 = r1;
			}
			pos--;
		}
		r2.next_reply = r1.next_reply;
		fseek(fp, rly_offset, SEEK_SET);
		fwrite(&r2, sizeof(struct reply), 1, fp);
		fflush(fp);
	}
	replyto_client("ok", csock);
	receive_data(csock);
}


void delete_all_replies(FILE *fp, int msg_offset){
	struct message m;
	struct reply r;
	int offset;
	fseek(fp, msg_offset, SEEK_SET);
	fread(&m, sizeof(struct message), 1, fp);
	offset = m.reply_offset;
	while (offset != 0){
		fseek(fp, offset, SEEK_SET);
		fread(&r, sizeof(struct reply), 1, fp);
		ClearBit_msg(offset, fp);
		offset = r.next_reply;
	}
}


void delete_message(FILE *fp, int cat_offset, int pos,int *csock){
	int temp_offset, msg_offset;
	struct category c;
	struct message m, m2;
	fseek(fp, cat_offset, SEEK_SET);
	fread(&c, sizeof(struct category), 1, fp);
	fseek(fp, c.msg_offset, SEEK_SET);
	fread(&m, sizeof(struct message), 1, fp);
	msg_offset = c.msg_offset;
	if (pos == 0){
		delete_all_replies(fp, c.msg_offset);
		c.msg_offset = m.next_msg;
		fseek(fp, cat_offset, SEEK_SET);
		fwrite(&c, sizeof(struct category), 1, fp);
		fflush(fp);
	}
	else{
		m2 = m;
		while (pos != 0){
			if (pos == 2)
				msg_offset = m.next_msg;
			fseek(fp, m.next_msg, SEEK_SET);
			fread(&m, sizeof(struct message), 1, fp);
			if (pos == 2)
				m2 = m;
			pos--;
		}
		delete_all_replies(fp, m.next_msg);
		m2.next_msg = m.next_msg;
		fseek(fp, msg_offset, SEEK_SET);
		fwrite(&m2, sizeof(struct message), 1, fp);
		fflush(fp);
	}
	replyto_client("ok", csock);
	receive_data(csock);
	return;
}


struct message add_reply(int offset_msg, FILE *fp,int *csock){
	int offset = find_empty_bit_msg(fp);
	struct message m;
	struct reply r;
	int ofs, ofs2,i;
	char *reply;
	fseek(fp, offset_msg, SEEK_SET);
	fread(&m, sizeof(struct message), 1, fp);
	if (m.reply_offset == 0){
		reply = receive_data(csock);
		replyto_client("ok", csock);
		for (i = 0; reply[i] != '\0'; i++)
			r.contents[i] = reply[i];
		r.contents[i] = '\0';
		for (i = 0; current_logged_user.name[i] != '\0'; i++)
			r.name_of_owner[i] = current_logged_user.name[i];
		r.name_of_owner[i] = '\0';
		r.next_reply = 0;
		m.reply_offset = offset;
		fseek(fp, offset_msg, SEEK_SET);
		fwrite(&m, sizeof(struct message), 1, fp);
		fseek(fp, offset, SEEK_SET);
		fwrite(&r, sizeof(struct reply), 1, fp);
		fflush(fp);
	}
	else{
		ofs = m.reply_offset;
		ofs2 = m.reply_offset;
		while (ofs != 0){
			fseek(fp, ofs, SEEK_SET);
			fread(&r, sizeof(struct reply), 1, fp);
			ofs = r.next_reply;
			if (r.next_reply != 0){
				ofs2 = r.next_reply;
			}
		}
		r.next_reply = offset;
		fseek(fp, ofs2, SEEK_SET);
		fwrite(&r, sizeof(struct reply), 1, fp);
		reply = receive_data(csock);
		replyto_client("ok", csock);
		for (i = 0; reply[i] != '\0'; i++)
			r.contents[i] = reply[i];
		r.contents[i] = '\0';
		for (i = 0; current_logged_user.name[i] != '\0'; i++)
			r.name_of_owner[i] = current_logged_user.name[i];
		r.name_of_owner[i] = '\0';
		r.next_reply = 0;
		fseek(fp, offset, SEEK_SET);
		fwrite(&r, sizeof(struct reply), 1, fp);
		fflush(fp);
	}
	//	fseek(fp, offset_msg, SEEK_SET);
	//	fwrite(&m, sizeof(struct message), 1, fp);
	return m;
}


int display_replies(int msg_offset, FILE *fp,int *csock){
	struct message m;
	struct reply r;
	int ch, ofs = 0, i = 0;
	char buff[4064], *reply;
	while (1){
		memset(buff, 0, 4064);
		buff[0] = '\0';
		fseek(fp, msg_offset, SEEK_SET);
		fread(&m, sizeof(struct message), 1, fp);
		i = 0;
		if (m.reply_offset == 0){
			replyto_client("", csock);
			reply = receive_data(csock);
			ch = reply[0];
			if (ch == 1){
				//printf("%d", sizeof(struct message));
				m = add_reply(msg_offset, fp,csock);
				fseek(fp, msg_offset, SEEK_SET);
				fwrite(&m, sizeof(struct message), 1, fp);
			}
			else{
				return 0;
			}
		}
		else{
			ofs = m.reply_offset;
			while (ofs != 0){
				fseek(fp, ofs, SEEK_SET);
				fread(&r, sizeof(struct reply), 1, fp);
				strcat(buff, "\n");
				strcat(buff, r.contents);
				strcat(buff, "\n");
				strcat(buff, r.name_of_owner);
				ofs = r.next_reply;
			}
			replyto_client(buff, csock);
			reply = receive_data(csock);
			ch = reply[0];
			if (ch == -2){
				m = add_reply(msg_offset, fp,csock);
				fseek(fp, msg_offset, SEEK_SET);
				fwrite(&m, sizeof(struct message), 1, fp);
			}
			else if (ch == -1){
				return 0;
			}
			else{
				delete_reply(fp, msg_offset, ch-1,csock);
			}
		}
	}
}


int add_messege(int offset_cat, FILE *fp,int *csock){
	//int offset = START_MSG + (MSG_SIZE * messeges);
	int offset = find_empty_bit_msg(fp);
	struct category c;
	struct message m, m1;
	int ofs, ofs2,i;
	char *reply, buff[4064];
	fseek(fp, offset_cat, SEEK_SET);
	fread(&c, sizeof(struct category), 1, fp);
	if (c.msg_offset == 0){
		c.msg_offset = offset;
		fseek(fp, offset_cat, SEEK_SET);
		fwrite(&c, sizeof(struct category), 1, fp);
		reply = receive_data(csock);
		replyto_client("ok", csock);
		for (i = 0; reply[i] != '\0'; i++)
			m.contents[i] = reply[i];
		m.contents[i] = '\0';
		for (i = 0; current_logged_user.name[i] != '\0'; i++)
			m.name_of_owner[i] = current_logged_user.name[i];
		m.name_of_owner[i] = '\0';
		m.next_msg = 0;
		m.reply_offset = 0;
	}
	else{
		ofs = c.msg_offset;
		ofs2 = c.msg_offset;
		while (ofs != 0){
			fseek(fp, ofs, SEEK_SET);
			fread(&m1, sizeof(struct message), 1, fp);
			ofs = m1.next_msg;
			if (m1.next_msg != 0){
				ofs2 = m1.next_msg;
			}
		}
		m1.next_msg = offset;
		fseek(fp, ofs2, SEEK_SET);
		fwrite(&m1, sizeof(struct message), 1, fp);
		fflush(fp);
		reply = receive_data(csock);
		replyto_client("ok", csock);
		for (i = 0; reply[i] != '\0'; i++)
			m.contents[i] = reply[i];
		m.contents[i] = '\0';
		for (i = 0; current_logged_user.name[i] != '\0'; i++)
			m.name_of_owner[i] = current_logged_user.name[i];
		m.name_of_owner[i] = '\0';
		m.next_msg = 0;
		m.reply_offset = 0;
	}
	fseek(fp, offset, SEEK_SET);
	fwrite(&m, sizeof(struct message), 1, fp);
	return 0;
}


int display_messages(int offset_cat, FILE *fp,int *csock){
	struct category c;
	struct message m;
	int ch, d,ofs,i=0;
	char *reply, buff[4064];
	while (1){
		buff[0] = '\0';
		fseek(fp, offset_cat, SEEK_SET);
		fread(&c, sizeof(struct category), 1, fp);
		if (c.msg_offset == 0){
			replyto_client(buff, csock);
			reply=receive_data(csock);
			ch = reply[0];
			if (ch == 1){
				add_messege(offset_cat, fp,csock);
			}
			else
				return 0;
		}
		else{
			ofs = c.msg_offset;
			i = 0;
			while (ofs != 0){
				fseek(fp, ofs, SEEK_SET);
				fread(&m, sizeof(struct message), 1, fp);
				strcat(buff, "\n");
				strcat(buff, m.contents);
				strcat(buff, "\n");
				strcat(buff, m.name_of_owner);
				i++;
				ofs = m.next_msg;
			}
			replyto_client(buff, csock);
			reply = receive_data(csock);
			ch = reply[0];
			if (ch == -1)
				return 0;
			else if (ch == -2)
				add_messege(offset_cat, fp, csock);
			else{
				replyto_client("ok", csock);
				reply = receive_data(csock);
				d = reply[0];
				switch (d){
				case 1:
					i = 0;
					ofs = c.msg_offset;
					while (i <= ch){
						if (i == ch){
							display_replies(ofs, fp, csock);
							break;
						}
						else{
							fseek(fp, ofs, SEEK_SET);
							fread(&m, sizeof(struct message), 1, fp);
							i++;
							ofs = m.next_msg;
						}
					}
					break;
				case 2:
					delete_message(fp, offset_cat, ch - 1, csock);
					break;
				}
			}
		}
	}
}


int add_category(int user_offset, FILE *fp,int *csock){
	int i,offset = CATEGORY_START + (CAT_STRUCT_SIZE * categories);
	struct category c;
	memset(&c, 0, 28);
	struct user_msg u;
	char *msg;
	fseek(fp, user_offset, SEEK_SET);
	fread(&u, sizeof(struct user_msg), 1, fp);
	if (u.no_of_cats > 5){
		replyto_client("", csock);
		receive_data(csock);
		return 0;
	}
	else{
		replyto_client("0", csock);
		receive_data(csock);
	}
	msg = receive_data(csock);
	replyto_client("ok", csock);
	for (i = 0; msg[i] != '\0'; i++)
		c.name[i] = msg[i];
	c.name[i] = '\0';
	c.no_of_msgs = 0;
	c.msg_offset = 0;
	categories++;
	fseek(fp, 0, SEEK_SET);
	fwrite(&categories, sizeof(int), 1, fp);
	fflush(fp);
	fseek(fp, offset, SEEK_SET);
	fwrite(&c, sizeof(struct category), 1, fp);
	if (u.no_of_cats < 0){
		u.offsets[0] = offset;
		u.no_of_cats = 1;
	}
	else{
		u.offsets[u.no_of_cats] = offset;
		u.no_of_cats = u.no_of_cats + 1;
	}
	fseek(fp, user_offset, SEEK_SET);
	fwrite(&u, sizeof(struct user_msg), 1, fp);
	fflush(fp);
	return offset;
}


int add_user(FILE *fp,char *name,int *csock){
	int ch, d,i;
	char *reply;
	struct user_msg temp;
	memset(&temp, 0, 40);
	int offset = USER_START + (user_count*USER_STRUCT_SIZE);
	for (i = 0; name[i] != '\0'; i++)
		temp.name[i] = name[i];
	temp.name[i] = '\0';
	temp.no_of_cats = 0;
	fseek(fp, offset, SEEK_SET);
	fwrite(&temp, sizeof(struct user_msg), 1, fp);
	fflush(fp);
	for (ch = 0; ch < 5; ch++){
		reply=receive_data(csock);
		replyto_client("ok", csock);
		d = reply[0];
		if (d == 1){
			add_category(offset, fp,csock);
			fseek(fp, offset, SEEK_SET);
			fread(&temp, sizeof(struct user_msg), 1, fp);
		}
		else
			break;
	}
	fseek(fp, offset, SEEK_SET);
	fwrite(&temp, sizeof(struct user_msg), 1, fp);
	fflush(fp);
	return offset;
}

void display_all_categories(FILE *fp, int *csock){
	int offset = CATEGORY_START, i, ch;
	struct category c;
	char buff[4064], *reply;
	while (1){
		buff[0] = '\0';
		for (i = 0; i < categories; i++){
			fseek(fp, (i*CAT_STRUCT_SIZE) + offset, SEEK_SET);
			fread(&c, sizeof(struct category), 1, fp);
			strcat(buff, "\n");
			strcat(buff, c.name);
		}
		replyto_client(buff, csock);
		reply = receive_data(csock);
		ch = reply[0];
		if (ch == -1)
			return;
		else{
			ch--;
			display_messages((ch*CAT_STRUCT_SIZE) + CATEGORY_START, fp, csock);
		}
	}
}


int display_categories(FILE *fp, int offset,int *csock){
	struct category c;
	struct user_msg temp;
	int i, ch;
	char buff[4064],*reply;
	fseek(fp, offset, SEEK_SET);
	fread(&temp, sizeof(temp), 1, fp);
	while (1){
		buff[0] = '\0';
		if (temp.no_of_cats == 0)
			return 0;
		for (i = 0; i < temp.no_of_cats; i++){
			fseek(fp, temp.offsets[i], SEEK_SET);
			fread(&c, sizeof(struct category), 1, fp);
			strcat(buff, "\n");
			strcat(buff, c.name);
		}
		replyto_client(buff, csock);
		reply=receive_data(csock);
		ch = reply[0];
		if (ch == -1)
			return 0;
		else
			display_messages(CATEGORY_START + CAT_STRUCT_SIZE*(ch-1), fp,csock);
	}
}


int display_users(FILE *fp,int *csock){
	char buff[4064], *reply;
	while (1){
		buff[0] = '\0';
		int ch, dh;
		struct user_msg temp;
		struct category c;
		int i, j;
		if (user_count > 0){
			for (i = 0; i < user_count; i++){
				fseek(fp, USER_START + (USER_STRUCT_SIZE * i), SEEK_SET);
				fread(&temp, sizeof(struct user_msg), 1, fp);
				strcat(buff, "\n");
				strcat(buff, temp.name);
			}
		}
		replyto_client(buff, csock);
		reply = receive_data(csock);
		dh = reply[0]-1;
		reply = receive_data(csock);
		ch = reply[0];
		switch (ch){
		case 1:
			display_all_categories(fp, csock);
			break;
		case 2:
			add_category(USER_START + (USER_STRUCT_SIZE * dh), fp,csock);
			break;
		case 3:
			return 0;
		default:
			printf("\n\nWrong option\n");
		}
	}
}


void msg_initialize(FILE *fp, int *csock){
	char *reply;
	int ch;
	while (1){
		reply = receive_data(csock);
		replyto_client("ok", csock);
		ch = reply[0];
		switch (ch){
		case 1:
			display_all_categories(fp, csock);
			break;
		case 2:
			display_categories(fp, current_logged_user.msgoffset, csock);
			break;
		case 3:
			add_category(current_logged_user.msgoffset, fp, csock);
			break;
		case 4:
			return;
		}
	}
}