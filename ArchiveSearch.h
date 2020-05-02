#ifndef ARCHIVESEARCH_H
#define ARCHIVESEARCH_H

#include <qstring.h>
#include <qlabel.h>
#include <qthread.h>

#include <kmainwindow.h>
#include <klistview.h>
#include <kpushbutton.h>
#include <kstatusbar.h>
#include <kregexp.h>
#include <kurlcombobox.h>
#include <kzip.h>
#include <ktar.h>

#define MAX_FILENAME_LENGTH	500

class SearchThread;

class ArchiveSearch: public KMainWindow
{
    Q_OBJECT
 public:
	ArchiveSearch(QWidget *parent,const char *name=0);
	~ArchiveSearch();
	
 public slots:
 	void Close();
 	void Search();
	void DirectoryChanged(const QString &string);
	void About();
	
 private:
 	int filesFound;
	char szfilename[MAX_FILENAME_LENGTH];
	bool useLikeOperator;
	volatile bool bSkipDir,bStopSearching;
	SearchThread *searchThread;
	
 	QLabel *lblFilename,*lblDirectory;
	KHistoryCombo *cboFilePattern;
	KURLComboBox *cboDirectory;
	KPushButton *pbSearch,*pbClose,*pbAbout;
	KListView *lvResult;
	KStatusBar *sbMain;
	KRegExp	searchPattern;
	
	void EnumerateDirectory(QString Directory);
	void EnumerateArchiveDirectory(const KArchiveDirectory* archiveDirectory,
									const QString& archiveFilename,	const QString& archivePath);
	bool IsLike(QString src, QString dest);
	friend class SearchThread;
};

class SearchThread: public QThread
{
	public:
		SearchThread(ArchiveSearch *archiveSearch);
		void run();
	private:
		ArchiveSearch *objArchiveSearch;
};


#endif

