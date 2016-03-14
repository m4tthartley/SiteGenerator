
char *O_Menu (menu m)
{
	char *output = PushMemory(KiloBytes(1));

	fiz (m.count) {
		sprintf(output, "%s<a href=\"%s\" class=\"menu%s\">%s</a> \n", output, m.items[i].dest, m.items[i].name, m.items[i].name);
	}

	return output;
}

void O_TemplateHeader (FILE *f, menu m)
{
	char *str = "<!DOCTYPE html>\n"
				"<html>"
				"<head>"
				"<title>%s</title>"
				"<meta name=\"description\" content=\"%s\">"
				"</head>"
				"<body>"
				"<div class=\"menu\">"
				"%s"
				"</div>"
				""
				""
				""
				""
				""
				""
				""
				""
				""
				""
				"";

	char *menuStr = O_Menu(m);
}