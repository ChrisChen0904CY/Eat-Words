#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <onnxruntime_cxx_api.h>
#include <QApplication>
#include <QActionGroup>
#include <QFileInfo>
#include <QSettings>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

#define SYNONYMS_TOLERANCE 0.7

/**

* @brief Include my own windows
* @author Chris Chan
* @date 2024/10/20

*/

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    /**

    * @brief Functions for initialization

    */
    void downloadFile(const QString &url, const QString &filePath);
    void onnx_download();
    void vocab_download();
    void rcs_check();
    void onnx_init();
    void vocab_init();
    void ini_check();
    void defaultSysParaSet();
    void sysParameterInit();
    void mode_init();

    /**

    * @brief Update the system parameters

    */
    // Apply a given system parameters
    void sysParameterApply(QMap<QString, QString> sysParam);
    // Update one single system parameter
    void sysParameterUpdate(QPair<QString, QString> param);

    /**

    * @brief Synonyms Judgement

    */
    bool synonyms_check(std::string& str1, std::string& str2,
                        float tol = SYNONYMS_TOLERANCE,
                        bool debug = false);

    /**

    * @brief Testers for particular libraries
    * @author Chris Chan
    * @date 2024/10/20

    */
    // Test for ONNX
    void onnx_test();

private slots:
    /**

    * @brief Slots for interacting with menubars and actions
    * @author Chris Chan
    * @date 2024/10/20

    */
    void mode_switch(QAction* action);

private:
    Ui::MainWindow *ui;
    /**

    * @memberof All variables related to the onnx
    * @author Chris Chan
    * @date 2024/10/20

    */
    Ort::Env* env = NULL;
    Ort::SessionOptions* session_options = NULL;
    Ort::Session* session = NULL;
    const wchar_t* model_path = L"./bert-base-chinese.onnx";
    QString model_link = "";
    // Vocabulary
    std::unordered_map<std::wstring, int> myVocab;
    std::string vocab_path = "./vocab.txt";
    QString vocab_link = "";

    /**

    * @memberof All variables related to the parameters initialization
    * @author Chris Chan
    * @date 2024/10/20

    */
    QString sysParameterFile = "./sysParameters.ini";
    // Default and current system parameters
    QMap<QString, QString> defaultSysParameters = {{"mode", "0"},
                                                   {"similarTol", "0.7"}};
    QMap<QString, QString> sysParameters = {};

    /**

    * @memberof All variables related to the mode action
    * @author Chris Chan
    * @date 2024/10/20

    */
    QActionGroup* modeGroup = NULL;
};

#endif // MAINWINDOW_H
