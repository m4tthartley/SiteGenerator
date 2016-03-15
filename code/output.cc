
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