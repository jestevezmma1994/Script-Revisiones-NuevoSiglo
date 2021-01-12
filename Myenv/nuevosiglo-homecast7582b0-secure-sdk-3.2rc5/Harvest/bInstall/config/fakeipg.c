#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <libxml/parser.h>

#include <svconf.h>

int fakeipg_get_channels_list(char *fname) {
  svconf_node_t *conf = NULL, *list = NULL, *lel = NULL, *lval = NULL;
  xmlDoc *doc = NULL;
  xmlNode *node = NULL, *child = NULL;
  char *p = NULL, buf[1024];
  int ret = -1, i;

  conf = svconf_load(NULL);
  if (!(list = svconf_lookup_object(conf, "CHANNELS", NULL))) {
    goto err;
  }
  if (!(doc = xmlNewDoc("1.0"))) goto err;
  node = xmlNewNode(NULL, "sense");

  xmlDocSetRootElement(doc, node);
  i = 0;
  for (lel = svconf_list_head(list, NULL); lel; 
       lel = svconf_list_get_next_elem(lel, NULL)) {
    if (!(lval = svconf_get_value(lel))) continue;
    i++;
    child = xmlNewChild(node, NULL, "channel", NULL);
    snprintf(buf, 1024, "%d", i);
    xmlSetProp(child, "number", buf);
    if ((p = svconf_lookup_string_object(lval, "NAME", NULL)))
      xmlNewTextChild(child, NULL, "title", p);
    if ((p = svconf_lookup_string_object(lval, "URL", NULL)))
      xmlNewTextChild(child, NULL, "url", p);
    if ((p = svconf_lookup_string_object(lval, "IMG", NULL)))
      xmlNewTextChild(child, NULL, "img", p);
  }
  if (xmlSaveFormatFile(fname, doc, 1) >= 0) ret = 0;
  xmlFreeDoc(doc);
 err:
  svconf_close(conf);
  return ret;
}


void fakeipg_add_item(xmlNode *node, char *page, char *title, char *desc, 
		    char *img, char *url, char *year, 
		    char *dir, char *cast, char *info) {
  xmlNode *p = NULL;
  p = xmlNewTextChild(node, NULL, "item", desc);
  if (page) xmlSetProp(p, "href", page);
  if (title) xmlSetProp(p, "title", title);
  if (img) xmlSetProp(p, "img", img);
  if (url) xmlSetProp(p, "play", url);
  if (year) xmlSetProp(p, "year", year);
  if (dir) xmlSetProp(p, "director", dir);
  if (cast) xmlSetProp(p, "cast", cast);
  if (info) xmlSetProp(p, "info", info);
}

int fakeipg_get_contents_list(char *fname, char *vname, char *cont_xml) {
  svconf_node_t *conf = NULL, *list = NULL, *lel = NULL, *lval = NULL;
  xmlDoc *doc = NULL;
  xmlNode *node = NULL;
  int ret = -1, i = 0;

  conf = svconf_load(NULL);
  if (!(list = svconf_lookup_object(conf, vname, NULL))) {
    goto err;
  }
  if (!(doc = xmlNewDoc("1.0"))) goto err;
  node = xmlNewNode(NULL, "list");
  xmlDocSetRootElement(doc, node);
  xmlSetProp(node, "pre", "10");
  xmlSetProp(node, "rowheight", "36");
  xmlSetProp(node, "color", "0x01f306ff");
  xmlSetProp(node, "focus", "color:0xffffffff");
//  xmlSetProp(node, "size", "7");

  for (lel = svconf_list_head(list, NULL); lel; 
       lel = svconf_list_get_next_elem(lel, NULL)) {
    if (!(lval = svconf_get_value(lel))) continue;
    i++;
    fakeipg_add_item(node, cont_xml, 
		       svconf_lookup_string_object(lval, "NAME", NULL),
		       svconf_lookup_string_object(lval, "DESC", NULL),
		       svconf_lookup_string_object(lval, "IMG", NULL),
		       svconf_lookup_string_object(lval, "URL", NULL),
		       svconf_lookup_string_object(lval, "YEAR", NULL),
		       svconf_lookup_string_object(lval, "DIRECTOR", NULL),
		       svconf_lookup_string_object(lval, "CAST", NULL),
		       svconf_lookup_string_object(lval, "INFO", NULL)
		     );
  }
  if (xmlSaveFormatFile(fname, doc, 1) >= 0) ret = 0;
  xmlFreeDoc(doc);
 err:
  svconf_close(conf);
  return ret;

}

void usage() {
  fprintf(stderr, "Usage: %s [--vod-list | --live-list | --tv-list ] fname\n\n",
	  "fakeipg");
}

int main(int argc, char **argv) {
  char *fname = NULL;

  if (argc < 2) {
    usage();
    return 1;
  }
  if (!strcmp(argv[1], "--vod-list")) {
    fname = argc > 2 ? argv[2] : "/tmp/content_list.xml";
    if (fakeipg_get_contents_list(fname, "VOD", "content/content.xml")) {
      fprintf(stderr, "Couldn't create VOD list\n");
      return 1;
    }
  } else if (!strcmp(argv[1], "--tv-list")) {
    fname = argc > 2 ? argv[2] : "/tmp/tv.xml";
    if (fakeipg_get_channels_list(fname)) {
      fprintf(stderr, "Couldn't create TV list\n");
      return 1;
    }
  } else if (!strcmp(argv[1], "--live-list")) {
    fname = argc > 2 ? argv[2] : "/tmp/content_list_live.xml";
    if (fakeipg_get_contents_list(fname, "LIVE", "content/content_live.xml")) {
      fprintf(stderr, "Couldn't create live list\n");
      return 1;
    }
  } else {
    usage();
    return 1;
  }
  return 0;
}

