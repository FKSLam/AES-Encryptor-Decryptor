#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include<stack>
#include<stdint.h>
#include<string.h>
using namespace std;
uint64_t total=0;
#define leftRotate(a,b) (a<<b)|(a>>(32u-b))
void galoisMultiply (unsigned char * factor, unsigned  char * original, int toMultiply)
{
	stack<bool> addOperation;
	for (int a=0; a<4; a++)
	{
		factor[a]=original[a];
	}
	// determine operations to be applied to each number
	while (toMultiply>1)
	{
		bool addRequired=(toMultiply%2==1);
		addOperation.push(addRequired);
		if (addRequired)
		toMultiply--;
		else toMultiply/=2;
	}
	while (!addOperation.empty())
	{
		bool addRequired=addOperation.top();
		addOperation.pop();
		// apply same operation to each number in original
		for (int a=0; a<4; a++)
		{
			if (!addRequired)
			{
				unsigned char bb=factor[a];
				unsigned char condition=(unsigned char)((signed char)(factor[a])>>7);
				factor[a]=factor[a]<<1;
				unsigned char tttemp=factor[a];
				factor[a]^=(0x1b&condition);
				unsigned char final=factor[a];
				unsigned char sss=0;
			}
			else factor[a]^=original[a];
		}
	}

	return;
}
void inverseMixColumns (unsigned char * col)
{
	// calculate 9x
	/*
	unsigned char mult9[4];
	for (int a=0; a<4; a++)
	{
		// shift 2 three times
		unsigned char condition = (unsigned char)((signed char)col[a]>>7);
		mult9[a]=col[a];
		for (int c=0; c<3; c++)
		{
			mult9[a]=mult9[a]<<1;
			mult9[a]^=(0x1b&condition);
			 condition = (unsigned char)((signed char)mult9[a]>>7);
		}
		// add one
		mult9[a]^=col[a];
	}*/
	unsigned char * mult9 = new unsigned char [4];
	galoisMultiply(mult9,col,9);
	unsigned char * mult11 = new unsigned char [4];
	galoisMultiply(mult11,col,11);
	unsigned char * mult13 = new unsigned char [4];
	galoisMultiply(mult13,col,13);
	unsigned char * mult14 = new unsigned char [4];
	galoisMultiply(mult14,col,14);
	col[0]=mult14[0]^mult11[1]^mult13[2]^mult9[3];
	col[1]=mult14[1]^mult11[2]^mult13[3]^mult9[0];
	col[2]=mult14[2]^mult11[3]^mult13[0]^mult9[1];
	col[3]=mult14[3]^mult11[0]^mult13[1]^mult9[2];
	delete[]mult9;
	delete[]mult11;
	delete[]mult13;
	delete[]mult14;
	return;
}
void AESDecryptByte(unsigned char *roundKeys, unsigned char *block, unsigned char **sTable)
{
	total++;
	 if (total%10000==0)cout << "Total Bytes Processed Currenty: " <<  total << endl;
	// do everything in reverse;
	/*
	unsigned char blockb[16]={0xff,0x0b,0x84,0x4a,0x08,0x53,0xbf,0x7c,0x69,0x34,0xab,0x43,0x64,0x14,0x8f,0xb9};
	unsigned char block[16];
	for (int d=0; d<4; d++)
	{
		for (int aa=0; aa<4; aa++)
		{
			block[4*aa+d]=blockb[4*d+aa];
		}
	}*/
	int currIndex=160;
	for (int a=0; a<10; a++)
	{
		// reverse add key, xor is removed through same operation
		for (int b=0; b<4; b++)
		{
			for (int c=0; c<4; c++)
			{
				block[4*c+b]=block[4*c+b]^roundKeys[4*b+c+currIndex];
			}
		}
		currIndex-=16;
		// reverse mixcolumns if not the first iteration
		if (a!=0)
		{
			unsigned char * col = new unsigned char [4];
			for (int b=0; b<4; b++)
			{
				for (int c=0; c<4; c++)
				{
					col[c]=block[4*c+b];
				}
				inverseMixColumns(col);
				for (int c=0; c<4; c++)
				{
					block[4*c+b]=col[c];
				}
			}
			delete[] col;
		}
		// reverse shift rows
		{
			// do second row
			unsigned char temp=block[7];
			for (int c=7; c>4;c--)
				block[c]=block[c-1];
			block[4]=temp;
			// do fourth row
			temp=block[12];
			for (int c=0; c<3;c++)
				block[12+c]=block[13+c];
			block[15]=temp;
			// do third row
			for (int c=0; c<2; c++)
			{
				temp=block[8+c];
				block[8+c]=block[10+c];
				block[10+c]=temp;
			}
		}
		// reverse subBytes
		unsigned char bitMask=0x0F;
		for (int ab=0; ab<16; ab++)
		{

			unsigned char col=block[ab]&bitMask;
			unsigned char row=(block[ab]>>4)&bitMask;
			block[ab]=sTable[row][col];

		}

	}
	// add round key one more time
	for (int b=0; b<4; b++)
	{
		for (int c=0; c<4; c++)
		{
			block[4*c+b]=block[4*c+b]^roundKeys[4*b+c+currIndex];
		}
	}
	/*
	cout << "ROUND " << "FINAL(DECRYPT)"<< endl;
		for (int cc=0; cc<16; cc++)
		{
			printf("%.2x ", block[cc]);
			if ((cc+1)%4==0) cout <<endl;
		}*/

}


bool initialize(unsigned char **invSTable,unsigned char **sTable, unsigned char * roundKeys, string initialKey)
{
	/*
	delete[] roundKeys;
	delete[]sTable;
	*/

	unsigned char * bytePointer;
	char input[200];
	ifstream in ("S_TABLE.txt");
	if (!in.is_open())
	{
		cout  << "Could not open file: S_TABLE.txt" << endl;
		return false;
	}
	stringstream hexConverter;
	//unsigned char roundKeys[176] ;
	// retrieve invsTable
	for (int a=0; a<16; a++)
	{
		int counter=2;
		invSTable[a]= new unsigned char [17];
		in.getline(input,200);
		for (int d=0; d<16;d++)
		{
			string b="0x";
			b=b+input[counter]+input[counter+1];
			hexConverter<< hex << b;
			unsigned int result = 0;

			hexConverter >> result;
			hexConverter.clear();
			hexConverter.str(string());
			invSTable[a][d]=result;
			counter+=5;
		}
	}

	// retrieve sTable
	for (int a=0; a<16; a++)
	{
		int counter=2;
		sTable[a]= new unsigned char [17];
		in.getline(input,200);
		for (int d=0; d<16;d++)
		{
			string b="0x";
			b=b+input[counter]+input[counter+1];
			hexConverter<< hex << b;
			unsigned int result = 0;

			hexConverter >> result;
			hexConverter.clear();
			hexConverter.str(string());
			sTable[a][d]=result;
			counter+=5;
		}
	}
	// retrieve rcon table
	unsigned char * rcon = new unsigned char [11];
	rcon[0]=141;
	rcon[1]=1;
	for (int aa=2; aa<9;aa++)
	{
		rcon[aa]=rcon[aa-1]<<1;
	}
	rcon[9]=27;
	rcon[10]=54;
	// generate roundKeys

	for (int a=0; a<16; a++)
	{

		unsigned char temp = 0;
		for (int cd=a*2; cd<a*2+2; cd++)
		{
			unsigned char ttemp=initialKey[cd];
			if (initialKey[cd]>='a') ttemp-=('a'-10);
			else if (initialKey[cd]>='A') ttemp-=('A'-10);
			else ttemp-=48;
			temp=(temp << 4)|ttemp;


		}
		roundKeys[a]=temp;
	}

	/*
	// DEBUG: Obtain key from file
	int counter=2;

		in.getline(input,200);
		for (int d=0; d<16;d++)
		{
			string b="0x";
			b=b+input[counter]+input[counter+1];
			hexConverter<< hex << b;
			unsigned int result = 0;

			hexConverter >> result;
			hexConverter.clear();
			hexConverter.str(string());
			roundKeys[d]=result;
			counter+=5;
		}*/
	int currIndex=16;
	int rconIterator=1;
	unsigned char bitMask=0x0F;
	for (int a=0; a<10; a++)
	{
		if (a==9)
		{
			int cc=0;
		}
		for (int cc=0;cc<4;cc++)
		{
			// create 4 bytes
			unsigned int temp=0;
			for (int d=0;d<4;d++)
			{
				unsigned char sss=roundKeys[currIndex-(4-d)];
				temp=temp|roundKeys[currIndex-(4-d)];
				if (d!=3)
					temp=temp<<8;
			}
			if (cc==0)
			{
				// key core schedule
				temp=leftRotate(temp,8);
				bytePointer=(unsigned char *)(&temp);
				// aply rijndael's sbox on four individual bytes
				for (int c=0;c<4;c++)
				{

						unsigned char ttemp=bytePointer[c];
						unsigned char col=ttemp&bitMask;
						unsigned char row=(ttemp>>4)&bitMask;
						unsigned char value=sTable[row][col];
						bytePointer[c]=sTable[row][col];

				}
				unsigned int toXOR = rcon[rconIterator]<<24;
				temp=temp^toXOR;
				rconIterator++;
			}
			unsigned int finalXor=0;
			for (int dd=0;dd<4;dd++)
			{
				finalXor=finalXor|roundKeys[currIndex-16+dd];
				if (dd!=3)
					finalXor=finalXor<<8;
			}
			temp=temp^finalXor;
			for (int dd=0; dd<4;dd++)
			{
				roundKeys[currIndex++]=(unsigned char)(temp>>(24-(8*dd)));
			}
		}

	}
	/*
	for (int a=0; a<16; a++)
	{
		for (int b=0; b<16; b++)
		{
			printf("%.2x ", sTable[a][b]);
		}
		cout << endl;
	}*/
	/*
	cout << "ROUND KEYS" << endl;

	for (int a=0; a<44; a++)
	{
		for (int b=0; b<4; b++)
		{
			printf("%.2x ", roundKeys[4*a+b]);
		}
		cout << endl;
		if (a&&(a+1)%4==0) cout << endl;
	}*/

	delete[] rcon;
	in.close();
	return true;
}
void mixColumns (unsigned char * col)
{
	// double matrix
	unsigned char doubleMatrix[4];
//	unsigned char * tMatrix = new unsigned char [4];

	//cout << endl;
	unsigned char original[4];
	for (int c=0; c<4; c++)
	{
		original[c]=col[c];
		unsigned char condition = (unsigned char)(((signed char)col[c])>>7);
		doubleMatrix[c]=col[c]<<1;
		doubleMatrix[c]^=(0x1b&condition);
	}
	/*
	galoisMultiply(tMatrix,col,2);
	 cout  << "TEST GALOIS MULTIPLY"<< endl;
	for (int cd=0; cd<4; cd++)
	{
		printf("%.2x", doubleMatrix[cd]);
	}
	cout << endl;
	for (int cd=0; cd<4; cd++)
	{
		printf("%.2x", tMatrix[cd]);
	}*/
	col[0]=doubleMatrix[0]^doubleMatrix[1]^original[1]^original[2]^original[3];
	col[1]=doubleMatrix[1]^doubleMatrix[2]^original[2]^original[0]^original[3];
	col[2]=doubleMatrix[2]^doubleMatrix[3]^original[3]^original[0]^original[1];
	col[3]=doubleMatrix[3]^doubleMatrix[0]^original[0]^original[1]^original[2];
	//delete[] tMatrix;

}
void AESEncryptByte(unsigned char * roundKeys, unsigned char * block, unsigned char **sTable)
{
	total++;
	if (total%10000==0) cout << "Total Bytes Processed Currenty: " <<  total << endl;
	// encrypts 16 bytes
	unsigned char * column = new unsigned char[4];
	//unsigned char block[16]={0x01,0x89,0xfe,0x76,0x23,0xAB,0xDC,0x54,0x45,0xcd,0xba,0x32,0x67,0xef,0x98,0x10};
	//unsigned char block[16]={0x00,0x44,0x88,0xcc,0x11,0x55,0x99,0xdd,0x22,0x66,0xAA,0xee,0x33,0x77,0xbb,0xff};
	// add round key (note: col major order)
	unsigned char bitMask=0x0F;
	for (int b=0; b<4; b++)
	{
		for (int c=0; c<4; c++)
		{
			unsigned char ttt=roundKeys[4*b+c];
			block[4*c+b]=block[4*c+b]^roundKeys[4*b+c];
		}

	}
	/*
	cout << "ROUND 0: " << endl;
	for (int cd=0; cd<16; cd++)
	{
		printf("%.2x ", block[cd]);
		if ((cd+1)%4==0) cout << endl;
	}
	cout << endl;*/
	for (int a=1; a<=10; a++)
	{
		// sub bytes
		for (int b=0; b<16; b++)
		{
			unsigned char col=block[b]&bitMask;
			unsigned char row=(block[b]>>4)&bitMask;

			block[b]=sTable[row][col];
		}

		// shift Rows
		{
			// do second row
			unsigned char temp=block[4];
			for (int c=0; c<3;c++)
				block[4+c]=block[5+c];
			block[7]=temp;
			// do fourth row
			temp=block[15];
			for (int c=0; c<3;c++)
				block[15-c]=block[14-c];
			block[12]=temp;
			// do third row
			for (int c=0; c<2; c++)
			{
				temp=block[8+c];
				block[8+c]=block[10+c];
				block[10+c]=temp;
			}
		}

		if (a!=10)
		{
			// perform mixcolumns
			for (int b=0; b<4; b++)
			{
				for (int aa=0; aa<4; aa++)
				{
					column[aa]=block[4*aa+b];
				}
				mixColumns(column);
				for (int aa=0; aa<4; aa++)
				{
					block[4*aa+b]=column[aa];
				}
			}
		}
		else
		{
			int gg=0;
		}

			/*cout << "Mix Columns FOR ROUND  " << a << ": "<< endl;
	for (int cd=0; cd<16; cd++)
	{
		printf("%.2x ", block[cd]);
		if ((cd+1)%4==0) cout << endl;
	}
	cout << endl;*/
		// add round key
		for (int b=0; b<4; b++)
		{
			for (int c=0; c<4; c++)
			{
				block[4*c+b]=block[4*c+b]^roundKeys[4*b+c+(16*a)];
			}

		}/*
			cout << "ROUND " << a << ": "<< endl;
	for (int cd=0; cd<16; cd++)
	{
		printf("%.2x ", block[cd]);
		if ((cd+1)%4==0) cout << endl;
	}
	cout << endl;*/
	}
	delete[] column;
}
void terminateProgram(unsigned char * roundKeys, unsigned char * block, unsigned char **sTable, unsigned char ** invSTable)
{

		if (roundKeys!=NULL)
		delete[] roundKeys;
	if (block!=NULL)
		delete[] block;
	if (sTable!=NULL)
	{
		for (int c=0; c<16; c++)
		{

				delete[] sTable[c];
		}
		delete[]sTable;
	}
	if (invSTable!=NULL)
	{
		for (int c=0; c<16; c++)
		{

				delete[] invSTable[c];
		}
		delete[]invSTable;
	}

}
string convertToHex(string charString)
{
	string toReturn="";
	int len=charString.length();
	for (int c=0;c<len&&c<16; c++)
	{
		for (int cc=0; cc<2; cc++)
		{
			unsigned char tt=charString[c];
			tt = tt >> (4-(4*cc));
			tt&=0x0F;
			if (tt>=10) tt+=55;
			else tt+=48;
			toReturn=toReturn+(char)tt;
		}
	}
	return toReturn;
}
int main()
{
	unsigned char ** sTable = new unsigned char *[16];
	unsigned char ** invSTable = new unsigned char *[16];
	unsigned char *roundKeys = new unsigned char [176];
	string initialKey="TEST";
	unsigned char * block;
	int option1;
	cout << "Type 1 for encryption, 0 for decryption" << endl;
	cin >> option1;
	cout << "Type the file to encrypt (include the extension)" <<endl;
	string fileName="e.txt";
	cin >>fileName;

	string outputName="decrypt.txt";
	cout <<"Type the desired name of the encrypted file (include extension)" << endl;
	cin >> outputName;
	char input[32];
	int option=0;
	cout << "Type 0 for 32 character hex key, 1 for 16 character key: ";
	cin >> option;
	if (!option)
	{
		cout << "Type a maximum 32 character key to use (note: key must be hexadecimal value, i.e. only characters 0-9 or A-F may be used)" << endl;
		cin >> initialKey;
	}
	else
	{
	    cout << "Type a maximum 16 character key to use: " << endl;
		cin >> initialKey;
		initialKey=convertToHex(initialKey);
	}
	//cin.getline(input,16);
	int diff=32-initialKey.length();
	for (int d=0; d<diff;d++)
		initialKey=initialKey+'0';
	//cin.getline(input,16);

	//cout << "TEST RETURN STRING: " << convertToHex("AAAAAAAAAAAAAAAB") << endl;
	//DEBUG
	//initialKey="0f1571c947d9e8590cb7add6af7f6798";
	// Note http://aesencryption.net/ treats key as a 'character string'
	//initialKey="41414141414141414141414141414142";
	ifstream toEncrypt;
	toEncrypt.open(fileName.c_str(), ios::binary|ios::in);
	ofstream encryptedFile;
	encryptedFile.open(outputName.c_str(),ios::binary|ios::out);
	//initialKey="0f1571c947d9e8590cb7add6af7f6798";

	if (toEncrypt.is_open())
	{
		if (encryptedFile.is_open())
		{
			if (!initialize(invSTable,sTable,roundKeys,initialKey))
			{
				cout << "INITIALIZATION failed" << endl;
				terminateProgram(roundKeys,block,sTable,invSTable);
				return 0;
			}


//	printf("%.2x\n", roundKeys[17]);
			toEncrypt.seekg(0,toEncrypt.end);
			uint64_t length=(uint64_t)toEncrypt.tellg();
			toEncrypt.seekg(0,toEncrypt.beg);
			// sacrifice memory to encode length of file, in 8 bytes (in beginning) in BIG ENDIAN
			if (option1)
			{
				// encode
				for (int cd=0; cd<8; cd++)
				{
					if (cd==7)
					{
						int tt=0;
					}
					unsigned char toWrite= length>>(56-(8*cd));
					encryptedFile.write((char*)(&toWrite),1);
				}
			}
			else
			{
				//decode
				length=0;
				char byteRetrieved[1];
				for (int cd=0; cd<8; cd++)
				{
					toEncrypt.read(byteRetrieved,1);
					length=length|(unsigned char)(byteRetrieved[0]);
					if (cd!=7)
					length<<=8;
				}
			}

			block = new unsigned char [16];
			int temp;
			uint64_t newLength=length;
		//	char input[16];
			while (toEncrypt.good())
			{
				memset(input,0,sizeof(input));
				toEncrypt.read(input,16);
				 temp=0;
				for (int cc=0; cc<toEncrypt.gcount(); cc++)
				{
					block[(temp%4)*4+(temp/4)]= (unsigned char)(input[cc]);
					temp++;
				}
				// test if block written properly
				/*
				cout << "BLOCK FILE TRACE TEST" << endl;
				for (int cc=0; cc<16; cc++)
				{
					printf("%.2x ", block[cc]);
					if ((cc+1)%4==0) cout << endl;
				}*/
				if (toEncrypt.gcount()!=16)
					break;
				if (option1)
				AESEncryptByte(roundKeys,block,sTable);
				else
				AESDecryptByte(roundKeys,block,invSTable);

				// write block to file
				int aTemp=0;

				//cout << "FINAL ANSWER" << endl;
				for (int cc=0; cc<16; cc++)
				{
					input[cc]=block[(aTemp%4)*4+(aTemp/4)];
					// write block as hex to standard output
					//printf("%.2x ", (unsigned char)input[cc]);
					aTemp++;
				}
				int toWrite=16;
				if (!option1&&newLength<(uint64_t)16) toWrite=newLength;

				encryptedFile.write(input,toWrite);
				if (!option1)
				{
					newLength-=(uint64_t)16;
					if (newLength<=0) break;
				}

			}
			if (length==0||(toEncrypt.gcount()!=0))
			{
				for (int a=toEncrypt.gcount(); a<16; a++)
				{
					block[(temp%4)*4+(temp/4)]=0;
					temp++;
				}
				if (option1)
				AESEncryptByte(roundKeys,block,sTable);
				else
				AESDecryptByte(roundKeys,block,invSTable);
				// write block to file
				int aTemp=0;
				for (int cc=0; cc<16; cc++)
				{
					input[cc]=block[(aTemp%4)*4+(aTemp/4)];
					// write block as hex to standard output
					//printf("%.2x ", (unsigned char)input[cc]);
					aTemp++;
				}
				encryptedFile.write(input,16);
			}
			toEncrypt.close();
			encryptedFile.close();
		}
		else
			cout << "Could not open file: " << outputName << endl;
	}
	else
		cout << "Could not open file: " << fileName << endl;

	terminateProgram(roundKeys,block,sTable,invSTable);
}
