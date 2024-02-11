#include <iostream>
#include <tagparser/abstractattachment.h>
#include <tagparser/diagnostics.h>
#include <tagparser/mediafileinfo.h>
#include <tagparser/progressfeedback.h>
#include <tagparser/tag.h>
#include <tagparser/tagvalue.h>

int main(int argc, char** argv)
{
	if(argc < 2)
		return 1;

	TagParser::MediaFileInfo fileInfo;
	TagParser::Diagnostics diag;
	TagParser::AbortableProgressFeedback prog;

	try
	{
		fileInfo.setPath(std::string(argv[1]));
		fileInfo.open();

		fileInfo.parseTags(diag, prog);

		std::vector<TagParser::Tag*> tags = fileInfo.tags();
		for(TagParser::Tag* tag : tags)
		{
			std::cout<<tag->value(TagParser::KnownField::Title).toString()<<'\n';
			std::cout<<tag->value(TagParser::KnownField::Artist).toString()<<'\n';
			std::cout<<tag->value(TagParser::KnownField::Album).toString()<<'\n';
		}
	}
	catch(std::ios_base::failure& err)
	{
		std::cout<<"std::ios_base::failure: "<<err.what()<<'\n';
		return 1;
	}



	return 0;
}
