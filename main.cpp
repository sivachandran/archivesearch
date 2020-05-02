/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Sat Feb 7 2004
    copyright            : (C) 2004 by C.P.Sivam
    email                : cpsivam@localhost.localdomain
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kapp.h>
#include <qlabel.h>
#include <qstring.h>
#include "ArchiveSearch.h"

int main(int argc,char **args)
{
	KApplication appArchiveSearch(argc,args,"Archive Search");

	ArchiveSearch *pArchiveSearch=new ArchiveSearch(NULL,"Archive Search");
	pArchiveSearch->show();
	appArchiveSearch.setMainWidget(pArchiveSearch);
	
	return(appArchiveSearch.exec());
	 
}
