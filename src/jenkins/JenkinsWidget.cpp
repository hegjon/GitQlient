#include "JenkinsWidget.h"

#include <RepoFetcher.h>
#include <JobContainer.h>
#include <JenkinsJobPanel.h>
#include <GitQlientSettings.h>
#include <GitBase.h>

#include <QTabWidget>
#include <QHBoxLayout>
#include <QNetworkAccessManager>

namespace Jenkins
{

JenkinsWidget::JenkinsWidget(const QSharedPointer<GitBase> &git, QWidget *parent)
   : QWidget(parent)
   , mGit(git)
   , mTabWidget(new QTabWidget())
{
   setObjectName("JenkinsWidget");

   GitQlientSettings settings;
   const auto url = settings.localValue(mGit->getGitQlientSettingsDir(), "BuildSystemUrl", "").toString();
   const auto user = settings.localValue(mGit->getGitQlientSettingsDir(), "BuildSystemUser", "").toString();
   const auto token = settings.localValue(mGit->getGitQlientSettingsDir(), "BuildSystemToken", "").toString();

   mConfig = IFetcher::Config { user, token, nullptr };
   mConfig.accessManager.reset(new QNetworkAccessManager());

   const auto layout = new QHBoxLayout(this);
   layout->setContentsMargins(10, 10, 10, 10);
   layout->setSpacing(10);
   layout->addWidget(mTabWidget);
   layout->addWidget(mPanel = new JenkinsJobPanel(mConfig));

   setMinimumSize(800, 600);

   mRepoFetcher = new RepoFetcher(mConfig, url);
   connect(mRepoFetcher, &RepoFetcher::signalViewsReceived, this, &JenkinsWidget::configureGeneralView);
}

void JenkinsWidget::reload() const
{
   mRepoFetcher->triggerFetch();
}

void JenkinsWidget::configureGeneralView(const QVector<JenkinsViewInfo> &views)
{
   GitQlientSettings settings;
   const auto user = settings.localValue(mGit->getGitQlientSettingsDir(), "BuildSystemUser", "").toString();
   const auto token = settings.localValue(mGit->getGitQlientSettingsDir(), "BuildSystemToken", "").toString();

   for (auto &view : views)
   {
      const auto container = new JobContainer(mConfig, view, this);
      container->setObjectName("JobContainer");
      connect(container, &JobContainer::signalJobInfoReceived, mPanel, &JenkinsJobPanel::onJobInfoReceived);
      mTabWidget->addTab(container, view.name);
   }
}

}