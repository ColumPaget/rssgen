title: rssgen
mansection: 1
date: 2016/11/23

NAME
----
rssgen - generate rss from a list of html documents.


DESCRIPTION
-----------
rssgen reads html documents and produces an rss feed that describes those documents. It gathers data in the following manner:

Document title is read from the "title" meta tag. If that's not present it's read from the "h1" header of the document. If that's not present the file name is used.
Document description is read from the "description" meta tag. if that's not present it's read from the "h2" header.
Document author is read from the "author" meta tag.
Publication date is read from the non-standard "pubDate" meta tag. If that's not present then the file modification time is used.


DISCLAIMER
----------
This is free software. It comes with no guarentees and I take no responsiblity if it makes your computer explode or opens a portal to the demon dimensions, or does anything.


INSTALL
--------
After unpacking the tarball do the usual "./configure; make; make install"
    cd rssgen-1.0
    ./configure
    make
    make install


SYNTAX
------
	rssgen <base url> -b [ -o <path> ] [ -t <title> ] [ -s <self url> ] [ -x <exclude pattern> ] <input file> ...

<base url>		The path that the items in the rss file will appear at. So, if building an rss for files under "http://myblog.com/blogposts/" the base url is "http://myblog.com/blogposts/"

*-b*
   :	Strip any leading path off input file names

*-o <path>*
   :	Path to RSS output file. Defaults to "./feed.rss"

*-t <title>*
   :	Title of RSS Channel

*-s <self url>*
   :  Optional "self" url for this RSS Channel

*-x <exclude pattern>*
   :  Don't include files matching this pattern


The first argument to rssgen must be the base URL where the documents will be stored. Thus this command

```
	rssgen http://mysite.org/blog/ -t "My Blog" post1.html post2.html
```

will generate an RSS file containing documents http://mysite.org/blog/post1.html and http://mysite.org/blog/post2.html.

Sometimes you may wish to build an RSS for files that are not in the current directory. For this the "-b" (basename) option exists. e.g. if you have website files in /home/webmaster/blog you might generate an rss with:

```
	rssgen http://mysite.org/blog/ -t "My Blog" -b /home/webmaster/blog/*.html
```

And this will strip the leading path from the html files and generate an RSS file containing documents http://mysite.org/blog/post1.html and http://mysite.org/blog/post2.html. Without the "-b" flag the paths will be included, so final URLs will be http://mysite.org/blog/home/webmaster/blog/post1.html etc.


META TAGS
---------
rssgen uses meta-tags within the document to get information about it. These are:

```
<meta name="title" content="Title for this document">
<meta name="description" content="Description of this document">
<meta name="author" content="author@somewhere.com">
<meta name="pubDate" content="Thu, 23 Nov 2016 20:43:16">
```

The "pubDate" meta-tag will also take dates in the following formats:

```
<meta name="pubDate" content="2016/11/23 20:43:16">
<meta name="pubDate" content="2016-11-23T20:43:16">
```

AUTHOR
------
rssgen is copyright (C) 2016 Colum Paget. They are released under the GPL so you may do anything with them that the GPL allows.

Email: colums.projects@gmail.com


