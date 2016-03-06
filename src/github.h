#pragma once

#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <functional>

class GitHubException : public std::exception
{
public:
  GitHubException(const QJsonObject &errorObj)
    : std::exception()
  {
    initMessage(errorObj);
  }

  virtual ~GitHubException() throw() override {}

  virtual const char *what() const throw()
  {
    return m_Message.constData();
  }

private:
  void initMessage(const QJsonObject &obj)
  {
    if (obj.contains("http_status")) {
      m_Message = QString("HTTP Status %1: %2")
                      .arg(obj.value("http_status").toInt())
                      .arg(obj.value("reason").toString())
                      .toUtf8();
    } else if (obj.contains("parse_error")) {
      m_Message = QString("Parsing failed: %1")
                      .arg(obj.value("parse_error").toString())
                      .toUtf8();
    } else if (obj.contains("network_error")) {
      m_Message = QString("Network failed: %1")
                      .arg(obj.value("network_error").toString())
                      .toUtf8();
    } else {
      m_Message = "Unknown error";
    }
  }

  QByteArray m_Message;
};

class GitHub : public QObject
{

  Q_OBJECT

public:
  enum class Method { GET, POST };

  struct Repository {
    Repository(const QString &owner, const QString &project)
      : owner(owner)
      , project(project)
    {
    }
    QString owner;
    QString project;
  };

public:
  GitHub(const char *clientId = nullptr);

  QJsonArray releases(const Repository &repo);
  void releases(const Repository &repo,
                const std::function<void (const QJsonArray &)> &callback);

private:
  QJsonDocument request(Method method, const QString &path,
                        const QByteArray &data, bool relative);
  void request(Method method, const QString &path, const QByteArray &data,
               const std::function<void (const QJsonDocument &)> &callback,
               bool relative);

  QJsonDocument handleReply(QNetworkReply *reply);
  QNetworkReply *genReply(Method method, const QString &path,
                          const QByteArray &data, bool relative);

private:
  QNetworkAccessManager *m_AccessManager;
};
