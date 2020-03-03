#include "MainWindow.h"

#include "Model/ModelThread.h"
#include "Model/Timesheet/Timesheet.h"
#include "Model/Timesheet/WorkDay.h"

#include <QAction>
#include <QDate>
#include <QDebug>

namespace View
{
	MainWindow::MainWindow( std::shared_ptr<Model::ModelThread> pModelThread_, QWidget* pParent_ )
		: QMainWindow( pParent_ ), m_pModelThread( pModelThread_ ), m_helpDialog( this )
	{
		m_ui.setupUi( this );

		setCurrentWeekLabel();
		m_workDays = { m_ui.mondayTime, m_ui.tuesdayTime, m_ui.wednesdayTime, m_ui.thursdayTime, m_ui.fridayTime };

		connect( m_ui.actionHelp, &QAction::triggered, [ this ]( const bool checked_ ) { OnHelpAction( checked_ ); } );
		connect( m_pModelThread.get(), &Model::ModelThread::TimesheetUpdated,
			[ this ]( const QSharedPointer<Model::Timesheet>& timesheet_ ) { OnTimesheetUpdated( *timesheet_ ); } );

		if ( m_pModelThread != nullptr )
		{
			m_pModelThread->start();
		}
	}

	/**
	 * @brief Callback when the close button is clicked. Ensure all threads are stopped.
	 */
	void MainWindow::closeEvent( QCloseEvent* /*pEvent_*/ )
	{
		qInfo() << "closing application...";
		m_pModelThread->stop();
		m_pModelThread->wait();
	}

	/*
	 * @brief Callback when the menu action button is clicked to open help dialog.
	 */
	void MainWindow::OnHelpAction( const bool /*checked_*/ )
	{
		m_helpDialog.open();
	}

	void MainWindow::setCurrentWeekLabel()
	{
		const QDate& currentDate = QDate::currentDate();
		const int currentWeekDay = currentDate.dayOfWeek();
		m_mondayDate = currentDate.addDays( static_cast<qint64>( -currentWeekDay ) + 1 ); // remove days to get monday
		m_fridayDate = m_mondayDate.addDays( 4 );										  // 1 + 4 = 5 = friday

		if ( m_ui.weekLabel != nullptr )
		{
			const QString dateFormat = "MMMM d yyyy"; // February 24 2020
			const QString newWeekLabel =
				m_mondayDate.toString( dateFormat ) + " - " + m_fridayDate.toString( dateFormat ) + ":";
			m_ui.weekLabel->setText( newWeekLabel );
		}
	}

	void MainWindow::OnTimesheetUpdated( const Model::Timesheet& timesheet_ )
	{
		if ( m_workDays.empty() )
		{
			return;
		}

		const Model::TWorkDays& workDays = timesheet_.GetWorkDays();
		for ( const auto& pWorkDay : workDays )
		{
			if ( pWorkDay == nullptr )
			{
				continue;
			}
			const QDate& date = pWorkDay->GetDate();
			if ( date < m_mondayDate )
			{
				continue;
			}
			QLineEdit* pWorkDayLineEdit = m_workDays.at( static_cast<size_t>( date.dayOfWeek() - 1 ) );
			if ( pWorkDayLineEdit != nullptr )
			{
				pWorkDayLineEdit->setText( pWorkDay->GetWorkTime().toString( "H'h'mm" ) );
			}
		}
	}
} // namespace View
