
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

void O_Page (FILE *f, page *p)
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
			case CONTENT_PARAGRAPH: {
				sprintf(s, "%s<p>", s);
				fiz (content->paragraph.wordCount) {
					if (content->paragraph.words[i].type == PARAGRAPH_WORD_WORD) {
						sprintf(s, "%s%s ", s, content->paragraph.words[i].str);
					} else if (content->paragraph.words[i].type == PARAGRAPH_WORD_LINK) {
						sprintf(s, "%s<a href=\"%s\">%s</a> ", s, content->paragraph.words[i].link.url, content->paragraph.words[i].link.str);
					}
				}
				sprintf(s, "%s</p>\n", s);
			} break;
		}
		fputs(s, f);

		content = content->next;
	}
}
