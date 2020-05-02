#include "ArchiveSearch.h"
#include <stdlib.h>
#include <string.h>

#include <qlayout.h>
#include <qptrlist.h>
#include <qfile.h>
#include <qdatastream.h>
#include <qvariant.h>
#include <qdir.h>

#include <kmessagebox.h>
#include <kdirselectdialog.h>
#include <kurlcompletion.h>
#include <kaboutdialog.h>

ArchiveSearch::ArchiveSearch(QWidget *parent,const char *name):KMainWindow(parent,name)
{
	QGridLayout *layout=new QGridLayout(this,4,4,5);
	setGeometry(0,0,600,400);
	
	lblFilename=new QLabel("Filename:",this);
	layout->addWidget(lblFilename,1,1);
	
	cboFilePattern=new KHistoryCombo(true,this);
	layout->addWidget(cboFilePattern,1,2);

	pbSearch=new KPushButton("Search",this);
	layout->addMultiCellWidget(pbSearch,1,1,3,4);

	lblDirectory=new QLabel("Directory:",this);
	layout->addWidget(lblDirectory,2,1);

	cboDirectory=new KURLComboBox(KURLComboBox::Directories,true,this);
	cboDirectory->setCompletionObject(new KURLCompletion());
	cboDirectory->setAutoDeleteCompletionObject(true); 

	const QFileInfoList* roots=QDir::drives();
	QPtrListIterator<QFileInfo> rootdrives(*roots);
	QFileInfo *fileCurrent;
	while((fileCurrent=*rootdrives))
	{
		++rootdrives;
		cboDirectory->insertItem(fileCurrent->filePath());
	}
	if(cboDirectory->count()==1 && cboDirectory->text(0))
	{
		QDir root=QDir::root();
		QPtrList<QFileInfo> filesInfo(*root.entryInfoList());

		for(fileCurrent=filesInfo.first();fileCurrent;fileCurrent=filesInfo.next())
			if(fileCurrent->fileName()!="." && fileCurrent->fileName()!="..")
				cboDirectory->insertItem(fileCurrent->filePath());
	}
	cboDirectory->insertItem("Browse...");
	cboDirectory->setDuplicatesEnabled(false);
	layout->addWidget(cboDirectory,2,2);

	pbAbout=new KPushButton("About",this);
	layout->addWidget(pbAbout,2,3);

	pbClose=new KPushButton("Close",this);
	layout->addWidget(pbClose,2,4);


	lvResult=new KListView(this);
	lvResult->addColumn("Filename",200);
	lvResult->addColumn("Archive Filename",200);
	lvResult->addColumn("Size",80);
	lvResult->setColumnAlignment(2,Qt::AlignRight);
	lvResult->addColumn("Directory",400);
	lvResult->setAllColumnsShowFocus(true);
	lvResult->setShowSortIndicator(true);
	lvResult->setResizeMode(QListView::LastColumn);
	layout->addMultiCellWidget(lvResult,3,3,1,4);

	sbMain=new KStatusBar(this);
  sbMain->insertItem("Ready",1,100,true);
  sbMain->setItemAlignment(1,Qt::AlignLeft);
	layout->addMultiCellWidget(sbMain,4,4,1,4);

	QObject::connect(pbSearch,SIGNAL(clicked()),this,SLOT(Search()));
	QObject::connect(pbClose,SIGNAL(clicked()),this,SLOT(Close()));
	QObject::connect(pbAbout,SIGNAL(clicked()),this,SLOT(About()));
	QObject::connect(cboDirectory,SIGNAL(activated(const QString&)),this,SLOT(DirectoryChanged(const QString&)));

	layout->activate();

	searchThread=new SearchThread(this);
}

bool ArchiveSearch::IsLike(QString src, QString dest)
{
	int iSrcPos=0,iDestPos=0,iSrcLen=src.length(),iDestLen=dest.length();

	src=src.upper();dest=dest.upper();
	while(iDestPos<iDestLen)
	{
		if(dest.at(iDestPos)=='*')
		{
			if(iDestPos==(iDestLen-1))
			{
				return true;
			}
			else
			{
				iDestPos++;
				while(src.at(iSrcPos)!=dest.at(iDestPos))
				{
					if(iSrcPos+1<iSrcLen)
						iSrcPos++;
					else
						return false;
				}
				if(src.at(iSrcPos)==dest.at(iDestPos))
				{
					if(iSrcPos+1<iSrcLen) iSrcPos++;
					if(iDestPos+1<iDestLen)	iDestPos++;
				}
			}
		}
		else
		{
			if(dest.at(iDestPos)=='?')
			{
				if(iSrcPos+1<iSrcLen) iSrcPos++;
				iDestPos++;
				continue;
			}
			else
			{

				if(src.at(iSrcPos)!=dest.at(iDestPos++))
					return false;
				if(iSrcPos+1<iSrcLen) iSrcPos++;
			}
		}
	}
	if(iSrcPos==(iSrcLen-1))
		return true;
	else
		return false;
}

void ArchiveSearch::Search()
{
	if(pbSearch->text()=="Search")
	{
		QString strDirectory(cboDirectory->currentText());
		if(!strDirectory.isNull() && !strDirectory.isEmpty())
		{
			QDir searchDir(strDirectory);
			if(searchDir.exists())
			{
				searchThread->start();
			}
			else
				KMessageBox::error(this,"Invalid Directory","Directory does not exists");
		}
	}
	else
		bSkipDir=true;
}

void ArchiveSearch::About()
{
	KAboutDialog* aboutDialog=new KAboutDialog(KAboutDialog::AbtKDEStandard,"About Archive Search",
																	KDialogBase::Ok,KDialogBase::Ok,this,"About Archive Search",true);
	aboutDialog->setCaption("About Archive Search");																	
	aboutDialog->setTitle("Archive Search 0.2");
 	//aboutDialog->setAuthor("Siva Chandran.P","scpmdu@yahoo.com",QString::null,QString::null);
  aboutDialog->addTextPage("Purpose","This is a small utility which is capable "
  														"of searching inside archives(zip,tar,tar.gz and tgz).",true);
  aboutDialog->addTextPage("Author","<center><b>Designed & Developed<br/>by</b><br/>"
  																	"Siva Chandran.P<br/><a href=\"mailto:scpmdu@yahoo.com\">"
                   									"scpmdu@yahoo.com</a></center>",true);
  aboutDialog->addTextPage("Contribution","Your feedback may help me to improve 'Archive Search'."
  														"So please feel free to send your feed back to "
                							"<a href=\"mailto:scpmdu@yahoo.com\">scpmdu@yahoo.com</a>",true);
	aboutDialog->setImageFrame(false);
	aboutDialog->exec();
}

void ArchiveSearch::EnumerateDirectory(QString Directory)
{
  QDir dirParent(Directory,QString::null,QDir::Name|QDir::DirsFirst,QDir::Dirs|QDir::Files|QDir::Readable);
	QPtrList<QFileInfo> filesInfo(*dirParent.entryInfoList("*.zip *.tar.gz",QDir::Files|QDir::Name));
	QFileInfo *fileCurrent;
	QString strFilePattern=cboFilePattern->currentText();
	KArchive* archiveFile;
	QString strExtension;

	for(fileCurrent=filesInfo.first();fileCurrent&&!bSkipDir&&!bStopSearching;fileCurrent=filesInfo.next())
	{
		if(!fileCurrent->isReadable()) continue;
		sbMain->changeItem(QString("%1").arg(fileCurrent->filePath()),1);

		strExtension=fileCurrent->extension().lower();
		if(strExtension.compare("zip")==0)
			archiveFile=new KZip(fileCurrent->filePath());
		else if((strExtension.compare("tar")==0) || (strExtension.compare("tar.gz")==0) || (strExtension.compare("tgz")==0))
			archiveFile=new KTar(fileCurrent->filePath());
		else
			continue;
						
		if(archiveFile->open(IO_ReadOnly))
		{
			EnumerateArchiveDirectory(archiveFile->directory(),fileCurrent->fileName(),fileCurrent->dirPath());				
			archiveFile->close();
		}				
	}
}

void ArchiveSearch::EnumerateArchiveDirectory(const KArchiveDirectory* archiveDirectory,
																							const QString& archiveFilename,
																							const QString& archivePath)
{
	QStringList archiveEntries=archiveDirectory->entries();
	QStringList::Iterator iterEntries=archiveEntries.begin();
	const KArchiveEntry* archiveEntry;
	long fileSize;
	QString strFilePattern=cboFilePattern->currentText();
	QString strFilename;
	QString strFilesize;
	bool addFilename;
	
	for(;iterEntries!=archiveEntries.end();++iterEntries)
	{
		archiveEntry=archiveDirectory->entry((*iterEntries));
		if(archiveEntry->isDirectory())
		{
			EnumerateArchiveDirectory((KArchiveDirectory*)archiveEntry,
																	archiveFilename+"/"+(*iterEntries),archivePath);
		}
		else
		{
			strFilename=*iterEntries;
			
			if(useLikeOperator)
				addFilename=IsLike(strFilename,strFilePattern);
			else
				addFilename=searchPattern.match(strFilename);

			if(addFilename)
			{
				fileSize=((KArchiveFile*)archiveEntry)->size();
				strFilesize.sprintf("%ld KB ",fileSize>1024?(int)((float)fileSize/1024):1);
				lvResult->insertItem(new QListViewItem(lvResult, *iterEntries, archiveFilename,
																		strFilesize,	archivePath));
				filesFound++;
			}
		}
	}
	
}

void ArchiveSearch::DirectoryChanged(const QString &string)
{
	if(string=="Browse...")
	{
		KDirSelectDialog selectDialog(QString::null,false,this,QString::null,true);
		QString dir=selectDialog.selectDirectory().directory();
		if(!dir.isEmpty())
		{
			if(cboDirectory->listBox()->findItem(dir,Qt::ExactMatch)==0)
				cboDirectory->insertItem(dir);
			cboDirectory->setCurrentText(dir);
		}
		else
			cboDirectory->setCurrentItem(0);
	}
}

void ArchiveSearch::Close()
{
	if(pbClose->text()=="Close")
		QWidget::close();
	else
		bStopSearching=true;
}

ArchiveSearch::~ArchiveSearch()
{
	if(searchThread->running()) searchThread->terminate();
	delete searchThread;
}

SearchThread::SearchThread(ArchiveSearch *archiveSearch)
{
	objArchiveSearch=archiveSearch;
}

void SearchThread::run()
{
	QString strDirectory(objArchiveSearch->cboDirectory->currentText());
	QString strFilePattern(objArchiveSearch->cboFilePattern->currentText());
	objArchiveSearch->filesFound=0;
	objArchiveSearch->bSkipDir=false;
	objArchiveSearch->bStopSearching=false;
	objArchiveSearch->pbSearch->setText("Skip Dir");
	objArchiveSearch->pbClose->setText("Stop");
	if(strFilePattern.contains('*')==0 && strFilePattern.contains('?')==0)
	{
		objArchiveSearch->useLikeOperator=false;
		objArchiveSearch->searchPattern.compile(strFilePattern,"i");
	}
	else
	{
		objArchiveSearch->useLikeOperator=true;
	}

	objArchiveSearch->lvResult->clear();
	objArchiveSearch->EnumerateDirectory(strDirectory);
	objArchiveSearch->sbMain->changeItem(QString("%1 file(s) found").arg(objArchiveSearch->filesFound,0,10),1);
	if(objArchiveSearch->filesFound>0)
  	objArchiveSearch->cboFilePattern->addToHistory(strFilePattern);
	objArchiveSearch->pbSearch->setText("Search");
	objArchiveSearch->pbClose->setText("Close");
}
