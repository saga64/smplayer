/*  smplayer, GUI front-end for mplayer.
    Copyright (C) 2006-2014 Ricardo Villalba <rvm@users.sourceforge.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "minigui.h"
#include "widgetactions.h"
#include "autohidewidget.h"
#include "myaction.h"
#include "mplayerwindow.h"
#include "global.h"
#include "helper.h"
#include "desktopinfo.h"
#include "editabletoolbar.h"
#include <QStatusBar>
#include <QMenu>

using namespace Global;

MiniGui::MiniGui( QWidget * parent, Qt::WindowFlags flags )
	: BaseGuiPlus( parent, flags )
{
	createActions();
	createControlWidget();
	createFloatingControl();

#if USE_CONFIGURABLE_TOOLBARS
	connect( editControlAct, SIGNAL(triggered()), controlwidget, SLOT(edit()) );
	EditableToolbar * iw = static_cast<EditableToolbar *>(floating_control->internalWidget());
	iw->takeAvailableActionsFrom(this);
	connect( editFloatingControlAct, SIGNAL(triggered()), iw, SLOT(edit()) );
#endif

	statusBar()->hide();

	retranslateStrings();

	loadConfig();

	if (pref->compact_mode) {
		controlwidget->hide();
	}
}

MiniGui::~MiniGui() {
	saveConfig();
}

#if USE_CONFIGURABLE_TOOLBARS
QMenu * MiniGui::createPopupMenu() {
	QMenu * m = new QMenu(this);
	m->addAction(editControlAct);
	m->addAction(editFloatingControlAct);
	return m;
}
#endif

void MiniGui::createActions() {
	timeslider_action = createTimeSliderAction(this);
	timeslider_action->disable();

#if USE_VOLUME_BAR
	volumeslider_action = createVolumeSliderAction(this);
	volumeslider_action->disable();
#endif

	time_label_action = new TimeLabelAction(this);
	time_label_action->setObjectName("timelabel_action");

	connect( this, SIGNAL(timeChanged(QString)),
             time_label_action, SLOT(setText(QString)) );

#if USE_CONFIGURABLE_TOOLBARS
	editControlAct = new MyAction( this, "edit_control_minigui" );
	editFloatingControlAct = new MyAction( this, "edit_floating_control_minigui" );
#endif
}


void MiniGui::createControlWidget() {
	controlwidget = new EditableToolbar( this );
	controlwidget->setObjectName("controlwidget");
	controlwidget->setLayoutDirection(Qt::LeftToRight);
	controlwidget->setMovable(true);
	controlwidget->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
	addToolBar(Qt::BottomToolBarArea, controlwidget);

#if USE_CONFIGURABLE_TOOLBARS
	QStringList controlwidget_actions;
	controlwidget_actions << "play_or_pause" << "stop" << "separator" << "timeslider_action" << "separator"
                          << "fullscreen" << "mute" << "volumeslider_action";
	controlwidget->setDefaultActions(controlwidget_actions);
#else
	controlwidget->addAction(playOrPauseAct);
	controlwidget->addAction(stopAct);
	controlwidget->addSeparator();
	controlwidget->addAction(timeslider_action);
	controlwidget->addSeparator();
	controlwidget->addAction(fullscreenAct);
	controlwidget->addAction(muteAct);
	#if USE_VOLUME_BAR
	controlwidget->addAction(volumeslider_action);
	#endif
#endif // USE_CONFIGURABLE_TOOLBARS
}

void MiniGui::createFloatingControl() {
	// Floating control
	floating_control = new AutohideWidget(panel);
	floating_control->setAutoHide(true);

	EditableToolbar * iw = new EditableToolbar(floating_control);
	iw->setObjectName("floating_control");

#if USE_CONFIGURABLE_TOOLBARS
	QStringList floatingcontrol_actions;
	floatingcontrol_actions << "play_or_pause" << "stop" << "separator" << "timeslider_action" << "separator"
                            << "fullscreen" << "mute";
	#if USE_VOLUME_BAR
	floatingcontrol_actions << "volumeslider_action";
	#endif
	floatingcontrol_actions << "separator" << "timelabel_action";
	iw->setDefaultActions(floatingcontrol_actions);
#else
	iw->addAction(playOrPauseAct);
	iw->addAction(stopAct);
	iw->addSeparator();
	iw->addAction(timeslider_action);
	iw->addSeparator();
	iw->addAction(fullscreenAct);
	iw->addAction(muteAct);
	#if USE_VOLUME_BAR
	iw->addAction(volumeslider_action);
	#endif
#endif // USE_CONFIGURABLE_TOOLBARS

	floating_control->setInternalWidget(iw);

#if !USE_CONFIGURABLE_TOOLBARS
	floating_control->adjustSize();
#endif

	floating_control->hide();
}

void MiniGui::retranslateStrings() {
	BaseGuiPlus::retranslateStrings();

	controlwidget->setWindowTitle( tr("Control bar") );

#if USE_CONFIGURABLE_TOOLBARS
	editControlAct->change( tr("Edit &control bar") );
	editFloatingControlAct->change( tr("Edit &floating control") );
#endif
}

#if AUTODISABLE_ACTIONS
void MiniGui::enableActionsOnPlaying() {
	BaseGuiPlus::enableActionsOnPlaying();

	timeslider_action->enable();
#if USE_VOLUME_BAR
	volumeslider_action->enable();
#endif
}

void MiniGui::disableActionsOnStop() {
	BaseGuiPlus::disableActionsOnStop();

	timeslider_action->disable();
#if USE_VOLUME_BAR
	volumeslider_action->disable();
#endif
}
#endif // AUTODISABLE_ACTIONS

void MiniGui::aboutToEnterFullscreen() {
	BaseGuiPlus::aboutToEnterFullscreen();

	floating_control->setMargin(pref->floating_control_margin);
	floating_control->setPercWidth(pref->floating_control_width);
	floating_control->setAnimated(pref->floating_control_animated);
	floating_control->setActivationArea( (AutohideWidget::Activation) pref->floating_activation_area);
	floating_control->setHideDelay(pref->floating_hide_delay);
	QTimer::singleShot(500, floating_control, SLOT(activate()));

	if (!pref->compact_mode) {
		controlwidget->hide();
	}
}

void MiniGui::aboutToExitFullscreen() {
	BaseGuiPlus::aboutToExitFullscreen();

	floating_control->deactivate();
	//floating_control->hide();

	if (!pref->compact_mode) {
		statusBar()->hide();
		controlwidget->show();
	}
}

void MiniGui::aboutToEnterCompactMode() {
	BaseGuiPlus::aboutToEnterCompactMode();

	controlwidget->hide();
}

void MiniGui::aboutToExitCompactMode() {
	BaseGuiPlus::aboutToExitCompactMode();

	statusBar()->hide();

	controlwidget->show();
}

#if USE_MINIMUMSIZE
QSize MiniGui::minimumSizeHint() const {
	return QSize(controlwidget->sizeHint().width(), 0);
}
#endif


void MiniGui::saveConfig() {
	QSettings * set = settings;

	set->beginGroup( "mini_gui");

	if (pref->save_window_size_on_exit) {
		qDebug("MiniGui::saveConfig: w: %d h: %d", width(), height());
		set->setValue( "pos", pos() );
		set->setValue( "size", size() );
		set->setValue( "state", (int) windowState() );
	}

	set->setValue( "toolbars_state", saveState(Helper::qtVersion()) );

#if USE_CONFIGURABLE_TOOLBARS
	set->beginGroup( "actions" );
	set->setValue("controlwidget", controlwidget->actionsToStringList() );
	EditableToolbar * iw = static_cast<EditableToolbar *>(floating_control->internalWidget());
	set->setValue("floating_control", iw->actionsToStringList() );
	set->endGroup();
#endif

	set->endGroup();
}

void MiniGui::loadConfig() {
	QSettings * set = settings;

	set->beginGroup( "mini_gui");

	if (pref->save_window_size_on_exit) {
		QPoint p = set->value("pos", pos()).toPoint();
		QSize s = set->value("size", size()).toSize();

		if ( (s.height() < 200) && (!pref->use_mplayer_window) ) {
			s = pref->default_size;
		}

		move(p);
		resize(s);

		setWindowState( (Qt::WindowStates) set->value("state", 0).toInt() );

		if (!DesktopInfo::isInsideScreen(this)) {
			move(0,0);
			qWarning("MiniGui::loadConfig: window is outside of the screen, moved to 0x0");
		}
	}

#if USE_CONFIGURABLE_TOOLBARS
	set->beginGroup( "actions" );
	controlwidget->setActionsFromStringList( set->value("controlwidget", controlwidget->defaultActions()).toStringList() );
	EditableToolbar * iw = static_cast<EditableToolbar *>(floating_control->internalWidget());
	iw->setActionsFromStringList( set->value("floating_control", iw->defaultActions()).toStringList() );
	floating_control->adjustSize();
	set->endGroup();
#endif

	restoreState( set->value( "toolbars_state" ).toByteArray(), Helper::qtVersion() );

	set->endGroup();
}

#include "moc_minigui.cpp"

