
char *O_Menu (menu m)
{
	char *output = PushMemory(KiloBytes(1));

	fiz (m.count) {
		sprintf(output, "%s<a href=\"%s\" class=\"menu%s\">%s</a> \n", output, m.items[i].dest, m.items[i].name, m.items[i].name);
	}

	return output;
}

void O_TemplateHeader (FILE *f, menu *m, char *style, page *p)
{
	char *str = "<!DOCTYPE html>\n"
				"<html>\n"
				"<head>\n"
				"<title>%s</title>\n"
				"<meta name=\"description\" content=\"%s\">\n"
				"<style type=\"text/css\">\n"
				"%s"
				"</style>\n"
				"</head>\n"
				"<body>\n"
				"<div class=\"menu\">\n"
				"%s"
				"</div>\n"
				"<div class=\"content\">\n";

	char *menuStr = O_Menu(*m);

	char *styleData = PushMemory(KiloBytes(10));
	s32 styleDataLength = strlen(style);
	s32 styleDataIndex = 0;
	fiz (styleDataLength) {
		if (style[i] != '\t' && style[i] != '\n') {
			styleData[styleDataIndex] = style[i];
			++styleDataIndex;
		} else {
			if (style[i] == '\n') {
				if (style[i-1] == '}' && style[i+1] != '}') {
					styleData[styleDataIndex] = style[i];
					++styleDataIndex;
				}
			}
		}
	}

	s32 headerSize = KiloBytes(10);
	char *header = PushMemory(headerSize);
	sprintf(header, str, p->Title, p->Desc, styleData, menuStr);
	fputs(header, f);
}

void O_TemplateFooter (FILE *f)
{
	char *str = "</div>\n"
				"</body>\n"
				"</html>\n"
				"";

	fputs(str, f);
}

void O_Page (FILE *f, page *p, page_list *blogList)
{
	page_content *content = p->content;
	while (content) {
		/*char s[64];
		sprintf(s, "%i\n", content->type);
		fputs(s, f);*/

		char *s = PushMemory(KiloBytes(10));
		switch (content->type) {
			case CONTENT_HEADER: {
				sprintf(s, "%s<h1>%s</h1>\n", s, content->header.str);
			} break;
			case CONTENT_SECONDARY_HEADER: {
				sprintf(s, "%s<h2>%s</h2>\n", s, content->header.str);
			} break;
			case CONTENT_PARAGRAPH: {
				sprintf(s, "%s<p>", s);
				fiz (content->paragraph.wordCount) {
					if (content->paragraph.words[i].type == PARAGRAPH_WORD_WORD) {
						if (content->paragraph.words[i].str[0] == '\\' && strlen(content->paragraph.words[i].str) == 1) {
							sprintf(s, "%s<br>", s);
						} else {
							if (content->paragraph.words[i].noSpace) {
								sprintf(s, "%s%s", s, content->paragraph.words[i].str);
							} else {
								sprintf(s, "%s %s", s, content->paragraph.words[i].str);
							}
						}
					} else if (content->paragraph.words[i].type == PARAGRAPH_WORD_LINK) {
						sprintf(s, "%s <a href=\"%s\">%s</a>", s, content->paragraph.words[i].link.url, content->paragraph.words[i].link.str);
					}
				}
				sprintf(s, "%s</p>\n", s);
			} break;
			case CONTENT_IMAGE: {
				sprintf(s, "%s<img src=\"/assets/%s\">\n<p class=\"caption\">%s</p>\n", s, content->image.fileName, content->image.caption);
			} break;
			case CONTENT_AUTHOR: {
				sprintf(s, "%s<p class=\"author\">by %s on %s</p>\n", s, content->author.str, GetPrintDate(p));
			} break;
			case CONTENT_BLOG_LIST: {
				fiz (blogList->count) {
					page *p = &blogList->pages[i];

					char *str = "%s"
								"<div class=\"blog\">\n"
								  "<div class=\"txt\">\n"
								    "<h2><a href=\"%s\">%s</a></h2>"
								    "<p>%s<br>%s</p>"
								  "</div>"
								  "<a href=\"%s\"><img src=\"/assets/%s\"></a>"
								"</div>";
					sprintf(s, str, s, p->FileName, p->Title, GetPrintDate(p), p->Desc, p->FileName, p->Image);
				}
			} break;
		}
		fputs(s, f);

		content = content->next;
	}
}

void O_BlogList (FILE *f, page_list *pageList)
{
	char *output = PushMemory(KiloBytes(10));

	fiz (pageList->count) {
		page *p = &pageList->pages[i];

		char *str = "%s"
					"<div class=\"blog\">\n"
					  "<div class=\"txt\">\n"
					    "<h2><a href=\"%s\">%s</a></h2>"
					    "<p>%s<br>%s</p>"
					  "</div>"
					  "<a href=\"%s\"><img src=\"/assets/%s\"></a>"
					"</div>";
		sprintf(output, str, output, p->FileName, p->Title, GetPrintDate(p), p->Desc, p->FileName, p->Image);
	}

	fputs(output, f);
}
