#ifndef KSYNC_OVERVIEWPART
#define KSYNC_OVERVIEWPART

#include <klocale.h>
#include <qpixmap.h>

#include <manipulatorpart.h>

namespace KSync {

  class OverviewWidget;

  class OverviewPart : public ManipulatorPart {
   Q_OBJECT
  public:
    OverviewPart(QWidget *parent, const char *name,
		 QObject *object=0, const char *name2 = 0, // make GenericFactory loading possible
		 const QStringList & = QStringList() );
    virtual ~OverviewPart();
    static KAboutData *createAboutData();

    QString type()const;
    QString name()const;
    QString description()const;
    bool partIsVisible()const;
    QPixmap *pixmap();
      QString iconName()const;
    QWidget* widget();

  private slots:
      void slotPartProgress( ManipulatorPart* part, const Progress& ) {

      };
      void slotPartError( ManipulatorPart*, const Error& ) {

      }
  private:
    QPixmap m_pixmap;
      QWidget* m_widget;
  };
};


#endif
