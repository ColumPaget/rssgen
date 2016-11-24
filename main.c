#include "libUseful-2.6/libUseful.h"
#include <glob.h>
#include <fnmatch.h>

#define RSS_CHANNEL 1

#define FLAG_STRIPPATH 1
#define FLAG_VERBOSE 2

typedef struct
{
int Type;
char *Title;
char *Description;
char *Link;
char *Self;
char *CopyRight;
char *Author;
time_t PubDate;
time_t LastBuildDate;
} TRSSItem;

typedef struct
{
int Flags;
char *Output;
char *WebRoot;
char *Exclude;
ListNode *Inputs;
ListNode *Items;
TRSSItem *Channel;
} TRSSContext;



void PrintUsage()
{
printf("rssgen <base url> -b [ -o <path> ] [ -t <title> ] [ -s <self url> ] [ -x <exclude pattern> ] <input file> ...\n\n");

printf("<base url>		The path that the items in the rss file will appear at. So, if building an rss for files under 'http://myblog.com/blogposts/' the base url is 'http://myblog.com/blogposts/'\n");
printf("-b		Strip any leading path off input file names\n");
printf("-o <path>		Path to RSS output file. Defaults to './feed.rss'\n");
printf("-t <title>	Title of RSS Channel.\n");
printf("-s <self url>  Optional 'self' url for this RSS Channel.\n");
printf("-x <exclude pattern>  Don't include files matching this pattern.\n");
printf("-v		Verbose output\n");
printf("-?	This help\n");
printf("-h	This help\n");
printf("-help	This help\n");
printf("--help 	This help\n");
}



TRSSContext *RSSCtxCreate(const char *WebRoot)
{
TRSSContext *Ctx;

Ctx=(TRSSContext *) calloc(1,sizeof(TRSSContext));
Ctx->Inputs=ListCreate();
Ctx->Items=ListCreate();
Ctx->Output=CopyStr(Ctx->Output, "feed.rss");
Ctx->Channel=(TRSSItem *) calloc(1,sizeof(TRSSItem));
Ctx->Channel->Type=RSS_CHANNEL;
Ctx->WebRoot=CopyStr(Ctx->WebRoot, WebRoot);	
Ctx->WebRoot=SlashTerminateDirectoryPath(Ctx->WebRoot);
Ctx->Channel->Link=CopyStr(Ctx->Channel->Link, Ctx->WebRoot);


return(Ctx);
}


TRSSContext *ParseCommandLine(int argc, char *argv[])
{
int i;
TRSSContext *Ctx;


if (argc < 3)
{
PrintUsage();
exit(1);
}

Ctx=RSSCtxCreate(argv[1]);
if (
		(strncasecmp(Ctx->WebRoot, "http:", 5) !=0) &&
		(strncasecmp(Ctx->WebRoot, "https:", 6) !=0)
)
{
	printf("Argument 1 must be a URL of the webroot of the website\n");
	exit(1);
}


for (i=2; i < argc; i++)
{
	if (strcmp(argv[i], "-o")==0)
	{
		i++;
		Ctx->Output=CopyStr(Ctx->Output, argv[i]);	
	}
	else if (strcmp(argv[i], "-t")==0) 
	{
		i++;
		Ctx->Channel->Title=CopyStr(Ctx->Channel->Title, argv[i]);	
	}
	else if (strcmp(argv[i], "-s")==0) 
	{
		i++;
		Ctx->Channel->Self=CopyStr(Ctx->Channel->Self, argv[i]);	
	}
	else if (strcmp(argv[i], "-x")==0) 
	{
		i++;
		Ctx->Exclude=CopyStr(Ctx->Exclude, argv[i]);	
	}
	else if (strcmp(argv[i], "-b")==0) Ctx->Flags |= FLAG_STRIPPATH;
	else if (strcmp(argv[i], "-v")==0) Ctx->Flags |= FLAG_VERBOSE;
	else if (strcmp(argv[i], "-?")==0) PrintUsage();
	else if (strcmp(argv[i], "-h")==0) PrintUsage();
	else if (strcmp(argv[i], "-help")==0) PrintUsage();
	else if (strcmp(argv[i], "--help")==0) PrintUsage();
	else ListAddNamedItem(Ctx->Inputs,argv[i],NULL);
}

return(Ctx);
}


void RSSOutputItem(STREAM *Out, TRSSItem *Item)
{
char *Tempstr=NULL;
time_t When;

//the '<channel>' and '</channel>' headers will be added outside of this function
if (Item->Type != RSS_CHANNEL) Tempstr=CopyStr(Tempstr,"<item>\n");

if (StrValid(Item->Title)) Tempstr=MCatStr(Tempstr, "<title>",Item->Title,"</title>\n",NULL);
if (StrValid(Item->Description)) Tempstr=MCatStr(Tempstr, "<description>",Item->Description,"</description>\n",NULL);
if (StrValid(Item->Author)) Tempstr=MCatStr(Tempstr, "<author>",Item->Author,"</author>\n",NULL);


//Only Channel elements have 'last build date'
if (Item->Type == RSS_CHANNEL)
{
if (Item->LastBuildDate) When=Item->LastBuildDate;
else time(&When);
Tempstr=MCatStr(Tempstr, "<lastBuildDate>",GetDateStrFromSecs("%a, %d %b %Y %H:%M:%S +0000",When,NULL), "</lastBuildDate>\n",NULL);
}

if (Item->PubDate) When=Item->PubDate;
else time(&When);
Tempstr=MCatStr(Tempstr, "<pubDate>",GetDateStrFromSecs("%a, %d %b %Y %H:%M:%S +0000",When,NULL), "</pubDate>\n",NULL);

if (StrLen(Item->Link)) Tempstr=MCatStr(Tempstr, "<link>",Item->Link,"</link>\n",NULL);
if (StrLen(Item->CopyRight)) Tempstr=MCatStr(Tempstr, "<copyright>",Item->CopyRight,"</copyright>\n",NULL);



if (Item->Type!=RSS_CHANNEL) Tempstr=CatStr(Tempstr,"</item>\n");

STREAMWriteLine(Tempstr, Out);
DestroyString(Tempstr);
}



/*
    <docs>http://blogs.law.harvard.edu/tech/rss</docs>^M
    <language>en-us</language>^M
    <managingEditor>marketing@feedforall.com</managingEditor>^M
    <webMaster>webmaster@feedforall.com</webMaster>^M
    <generator>FeedForAll Beta1 (0.0.1.8)</generator>^M
    <image>^M
      <url>http://www.feedforall.com/ffalogo48x48.gif</url>^M
      <title>FeedForAll Sample Feed</title>^M
      <link>http://www.feedforall.com/industry-solutions.htm</link>^M
      <description>FeedForAll Sample Feed</description>^M
      <width>48</width>^M
      <height>48</height>^M
    </image>^M
*/


void ProcessMetaTag(TRSSItem *Item, const char *MetaTag)
{
char *Name=NULL, *Value=NULL;
char *MetaName=NULL, *MetaValue=NULL;
const char *ptr;

ptr=GetNameValuePair(MetaTag, " ", "=",&Name,&Value);
while (ptr)
{
	if (strcasecmp(Name,"name")==0) MetaName=CopyStr(MetaName, Value);
	if (strcasecmp(Name,"content")==0) MetaValue=CopyStr(MetaValue, Value);
	
	ptr=GetNameValuePair(ptr, " ", "=",&Name,&Value);
}

if (StrValid(MetaName))
{
	if (strcasecmp(MetaName, "title")==0) Item->Title=CopyStr(Item->Title, MetaValue);
	else if (strcasecmp(MetaName, "description")==0) Item->Description=CopyStr(Item->Description, MetaValue);
	else if (strcasecmp(MetaName, "author")==0) Item->Author=CopyStr(Item->Author, MetaValue);
	else if (strcasecmp(MetaName, "pubDate")==0) 
	{
		Item->PubDate=DateStrToSecs("%a, %d %b %Y %H:%M:%S",MetaValue,NULL);
		if (Item->PubDate==-1) Item->PubDate=DateStrToSecs("%a, %Y %m %d %H:%M:%S",MetaValue,NULL);
		if (Item->PubDate==-1) Item->PubDate=DateStrToSecs("%Y-%m-%d %H:%M:%S",MetaValue,NULL);
		if (Item->PubDate==-1) Item->PubDate=DateStrToSecs("%Y %m %d %H:%M:%S",MetaValue,NULL);
	}
}

DestroyString(Name);
DestroyString(Value);
DestroyString(MetaName);
DestroyString(MetaValue);
}

void ProcessHtmlFile(TRSSItem *Item, STREAM *S)
{
char *Tempstr=NULL, *Key=NULL, *Value=NULL, *ptr;

	Tempstr=STREAMReadLine(Tempstr, S);
	while (Tempstr)
	{
		ptr=XMLGetTag(Tempstr, NULL, &Key, &Value); 
		while (ptr)
		{
		switch (*Key)
		{
		case 't':
		case 'T':
		if (strcasecmp(Key, "title"	)==0) ptr=XMLGetTag(ptr, NULL, &Key, &Item->Title); 
		break;

		case 'm':
		case 'M':
		if (strcasecmp(Key, "meta")==0) ProcessMetaTag(Item, Value); 
		break;

		case 'h':
		case 'H':
		if ((! StrValid(Item->Title)) && (strcasecmp(Key, "h1")==0)) ptr=XMLGetTag(ptr, NULL, &Key, &Item->Title); 
		if ((! StrValid(Item->Description)) && (strcasecmp(Key, "h2")==0)) ptr=XMLGetTag(ptr, NULL, &Key, &Item->Description); 
		break;
		}

		ptr=XMLGetTag(ptr, NULL, &Key, &Value);
		}
	Tempstr=STREAMReadLine(Tempstr, S);
	}

DestroyString(Tempstr);
DestroyString(Value);
DestroyString(Key);
}


void RSSProcessFile(TRSSContext *RSS, const char *Path)
{
char *Tempstr=NULL;
TRSSItem *Item;
struct stat Stat;
STREAM *S;

S=STREAMOpenFile(Path,SF_RDONLY);
if (S)
{
	stat(Path,&Stat);
	Item=(TRSSItem *) calloc(1, sizeof(TRSSItem));
	if (RSS->Flags & FLAG_STRIPPATH) Item->Link=MCopyStr(Item->Link, RSS->WebRoot, GetBasename(Path), NULL);
	else Item->Link=MCopyStr(Item->Link, RSS->WebRoot, Path, NULL);
	Item->PubDate=Stat.st_ctime;
	Item->LastBuildDate=Stat.st_mtime;
	ListAddItem(RSS->Items, Item);
	ProcessHtmlFile(Item, S);
	if (RSS->Flags & FLAG_VERBOSE) 
	{
			printf("add: %s\n",Path);
			printf("title: %s\n",Item->Title);
			if (StrLen(Item->Author)) printf("author: %s\n",Item->Author);
			printf("pubdate: %s\n",GetDateStrFromSecs("%Y/%m/%d %H:%M:%S", Item->PubDate,NULL));
			printf("\n");
	}
	STREAMClose(S);
}

DestroyString(Tempstr);
}


void RSSProcessDirectory(TRSSContext *RSS, const char *Path)
{
char *Tempstr=NULL;
glob_t Glob;
int i;

Tempstr=MCopyStr(Tempstr, Path, "/*", NULL);
glob(Tempstr, 0, 0, &Glob);
for (i=0; i < Glob.gl_pathc; i++)
{
	RSSProcessFile(RSS, Glob.gl_pathv[i]);
}
globfree(&Glob);

DestroyString(Tempstr);
}


STREAM *RSSOpenOutput(TRSSContext *Ctx)
{
STREAM *S;

S=STREAMOpenFile(Ctx->Output,SF_CREAT | SF_TRUNC | SF_WRONLY);
if (! S) return(S);

STREAMWriteLine("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n",S);
STREAMWriteLine("<rss version=\"2.0\" xmlns:atom=\"http://www.w3.org/2005/Atom\">\n",S);
STREAMWriteLine("<channel>\n",S);

RSSOutputItem(S, Ctx->Channel);

return(S);
}


void RSSCloseOutput(STREAM *S)
{
STREAMWriteLine("</channel>\n",S);
STREAMWriteLine("</rss>\n",S);
STREAMClose(S);
}


main(int argc, char *argv[])
{
ListNode *Curr;
struct stat Stat;
TRSSContext *RSS;
STREAM *Out;

RSS=ParseCommandLine(argc, argv);
Curr=ListGetNext(RSS->Inputs);
while (Curr)
{
	if (StrValid(RSS->Exclude) && 
			(
			(fnmatch(RSS->Exclude, Curr->Tag, 0) == 0) ||
			(fnmatch(RSS->Exclude, GetBasename(Curr->Tag), 0) == 0)
			)
		)
	{
		//exclude this item
		if (RSS->Flags & FLAG_VERBOSE) printf("excluding: %s\n",Curr->Tag);
	}
	else
	{
	stat(Curr->Tag,&Stat);
	if (S_ISDIR(Stat.st_mode)) RSSProcessDirectory(RSS,Curr->Tag);
	else RSSProcessFile(RSS,Curr->Tag);
	}
Curr=ListGetNext(Curr);
}

Out=RSSOpenOutput(RSS);
Curr=ListGetNext(RSS->Items);
while (Curr)
{
	RSSOutputItem(Out, (TRSSItem *) Curr->Item);
	Curr=ListGetNext(Curr);
}
RSSCloseOutput(Out);
}
