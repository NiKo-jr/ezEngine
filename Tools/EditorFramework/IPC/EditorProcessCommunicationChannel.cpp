﻿#include <PCH.h>
#include <EditorFramework/IPC/EditorProcessCommunicationChannel.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Communication/IpcChannel.h>
#include <Foundation/Logging/Log.h>
#include <QCoreApplication>
#include <QProcess>

ezResult ezEditorProcessCommunicationChannel::StartClientProcess(const char* szProcess, const QStringList& args, bool bRemote, const ezRTTI* pFirstAllowedMessageType, ezUInt32 uiMemSize)
{
  EZ_LOG_BLOCK("ezProcessCommunicationChannel::StartClientProcess");

  EZ_ASSERT_DEV(m_pChannel == nullptr, "ProcessCommunication object already in use");
  EZ_ASSERT_DEV(m_pClientProcess == nullptr, "ProcessCommunication object already in use");

  m_pFirstAllowedMessageType = pFirstAllowedMessageType;

  static ezUInt64 uiUniqueHash = 0;
#if EZ_ENABLED(EZ_PLATFORM_WINDOWS)
  DWORD PID = GetCurrentProcessId();
  uiUniqueHash = ezHashing::MurmurHash64(&PID, sizeof(PID), uiUniqueHash);
#endif
  ezTime time = ezTime::Now();
  uiUniqueHash = ezHashing::MurmurHash64(&time, sizeof(time), uiUniqueHash);
  ezStringBuilder sMemName;
  sMemName.Format("{0}", ezArgU(uiUniqueHash, 16, false, 16, true));
  ++uiUniqueHash;

  if (bRemote)
  {
    m_pChannel = ezIpcChannel::CreateNetworkChannel("localhost:1050", ezIpcChannel::Mode::Client);
  }
  else
  {
    m_pChannel = ezIpcChannel::CreatePipeChannel(sMemName, ezIpcChannel::Mode::Server);
  }

  m_pChannel->m_MessageEvent.AddEventHandler(ezMakeDelegate(&ezProcessCommunicationChannel::MessageFunc, this));
  m_pChannel->Connect();

  ezStringBuilder sPath = szProcess;

  if (!sPath.IsAbsolutePath())
  {
    sPath = ezOSFile::GetApplicationDirectory();
    sPath.AppendPath(szProcess);
  }

  sPath.MakeCleanPath();

  ezStringBuilder sPID;
  ezConversionUtils::ToString((ezUInt64)QCoreApplication::applicationPid(), sPID);

  QStringList arguments;
  arguments << "-IPC";
  arguments << QLatin1String(sMemName.GetData());
  arguments << "-PID";
  arguments << sPID.GetData();
  arguments.append(args);

  m_pClientProcess = new QProcess();
  m_pClientProcess->start(QString::fromUtf8(sPath.GetData()), arguments, QIODevice::OpenModeFlag::NotOpen);

  if (!m_pClientProcess->waitForStarted())
  {
    delete m_pClientProcess;
    m_pClientProcess = nullptr;

    EZ_DEFAULT_DELETE(m_pChannel);

    ezLog::Error("Failed to start process '{0}'", sPath.GetData());
    return EZ_FAILURE;
  }

  return EZ_SUCCESS;
}

bool ezEditorProcessCommunicationChannel::IsClientAlive() const
{
  if (m_pClientProcess == nullptr)
    return false;

  bool bRunning = m_pClientProcess->state() != QProcess::NotRunning;
  bool bNoError = m_pClientProcess->error() == QProcess::UnknownError;

  return bRunning && bNoError;
}

void ezEditorProcessCommunicationChannel::CloseConnection()
{
  EZ_DEFAULT_DELETE(m_pChannel);

  if (m_pClientProcess)
  {
    m_pClientProcess->close();
    delete m_pClientProcess;
    m_pClientProcess = nullptr;
  }
}