#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utils.h"
#include "loadingwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Initialize the loadingWindow
    LoadingWindow* loadingWindow = new LoadingWindow(this);
    loadingWindow->show();
    loadingWindow->set_label_text("欢迎使用Eat Words！");
    // Delay 100ms to display the loading "animation"
    QTimer::singleShot(100, this, [this, loadingWindow]{
        // Check the resources first
        loadingWindow->set_label_text("资源文件检查中...");
        QCoreApplication::processEvents();
        rcs_check();
        // Load the model in
        loadingWindow->set_label_text("Bert模型加载中...");
        QCoreApplication::processEvents();
        this->onnx_init();
        // Load the vocabulary in
        loadingWindow->set_label_text("分词词汇表加载中...");
        QCoreApplication::processEvents();
        this->vocab_init();
        // Load the mode initial setting in
        loadingWindow->set_label_text("加载系统预设参数...");
        QCoreApplication::processEvents();
        this->ini_check();
        this->sysParameterInit();
        this->mode_init();
        // Show the MainWindow after finish loading the model
        loadingWindow->close();
        this->show();
        // Run the testers
        onnx_test();
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

/**

* @brief Functions for initialization
* @author Chris Chan
* @date 2024/10/20

*/
void MainWindow::downloadFile(const QString &url, const QString &filePath) {
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.get(QNetworkRequest(QUrl(url)));

    // Waiting for the downloading
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() == QNetworkReply::NoError) {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(reply->readAll());
            file.close();
            qDebug() << "文件下载成功:" << filePath;
        } else {
            qDebug() << "无法保存文件:" << file.errorString();
        }
    } else {
        qDebug() << "下载错误:" << reply->errorString();
    }

    reply->deleteLater();
}

void MainWindow::rcs_check() {
    // Check the model file
    if (!QFile::exists(QString::fromStdWString(model_path))) {
        onnx_download();
    }
    // Check the vocabulary text file
    if (!QFile::exists(QString::fromStdString(vocab_path))) {
        vocab_download();
    }
}

void MainWindow::onnx_download() {

}

void MainWindow::vocab_download() {

}

void MainWindow::onnx_init() {
    // Create the ONNX Runtime Enviroment
    env = new Ort::Env(ORT_LOGGING_LEVEL_WARNING, "ONNXModel");

    // Create the Session Options
    session_options = new Ort::SessionOptions();
    session_options->SetIntraOpNumThreads(1);
    session_options->SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_BASIC);

    // Load the ONNX Model
    try {
        session = new Ort::Session(*env, model_path, *session_options);
    } catch (const Ort::Exception& e) {
        qDebug() << "Failed to create ONNX Runtime session: " << e.what();
    }
}

void MainWindow::vocab_init() {
    // Load the vocabulary in
    myVocab = load_vocab(vocab_path);
}

// Check the system initialization file
void MainWindow::ini_check() {
    QFileInfo sys_int_file(this->sysParameterFile);
    // Create a new file while not existed
    if (!sys_int_file.isFile()) {
        this->defaultSysParaSet();
    }
}

// Set the Default System Parameters
void MainWindow::defaultSysParaSet() {
    // Open the system parameters ini file
    QSettings*  m_IniFile = new QSettings(this->sysParameterFile, QSettings::IniFormat);

    for(auto it = defaultSysParameters.constBegin(); it != defaultSysParameters.constEnd(); ++it) {
        this->sysParameters[it.key()] = it.value();
        m_IniFile->setValue("sysParam/"+it.key(), it.value());
    }

    delete m_IniFile;
}

// Initialize the system parameters
void MainWindow::sysParameterInit() {
    // Open the system parameters ini file
    QSettings*  m_IniFile = new QSettings(this->sysParameterFile, QSettings::IniFormat);

    for(auto it = defaultSysParameters.constBegin(); it != defaultSysParameters.constEnd(); it++) {
        try {
            sysParameters[it.key()] = m_IniFile->value("sysParam/"+it.key()).toString();
        }
        catch(const char* msg) {
            qDebug() << msg;
            defaultSysParaSet();
        }
        if(sysParameters[it.key()]=="") {
            sysParameters[it.key()] = it.value();
        }
    }

    delete m_IniFile;
}

void MainWindow::mode_init() {
    // Initialize the action group first
    modeGroup = new QActionGroup(this);
    // Put all actions belong to the mode bar into the group
    for (auto action : this->ui->menuMode->actions()) {
        modeGroup->addAction(action);
    }
    // Set this group exclusive
    modeGroup->setExclusive(true);

    // Connect the action group to a slot controlling of the switch of mode
    connect(this->modeGroup,
            &QActionGroup::triggered,
            this,
            &MainWindow::mode_switch);

    // Call the mode_switch immediately for correct setting
    switch (sysParameters["mode"].toInt()) {
    case 0:
        this->ui->actionSearch->setChecked(true);
        mode_switch(this->ui->actionSearch);
        break;
    case 1:
        this->ui->actionIELTS->setChecked(true);
        mode_switch(this->ui->actionIELTS);
        break;
    case 2:
        this->ui->actionGRE->setChecked(true);
        mode_switch(this->ui->actionGRE);
        break;
    default:
        this->ui->actionSearch->setChecked(true);
        mode_switch(this->ui->actionSearch);
        break;
    }
}

/**

* @brief Update the system parameters

*/
// Apply a given system parameters
void MainWindow::sysParameterApply(QMap<QString, QString> sysParam) {
    sysParameters = sysParam;
    QSettings*  m_IniFile = new QSettings(this->sysParameterFile, QSettings::IniFormat);
    for (auto it = sysParam.constBegin(); it != sysParam.constEnd(); ++it) {
        m_IniFile->setValue("sysParam/" + it.key(), it.value());
    }
    delete m_IniFile;
}

// Update one single system parameter
void MainWindow::sysParameterUpdate(QPair<QString, QString> param) {
    sysParameters[param.first] = param.second;
    QSettings*  m_IniFile = new QSettings(this->sysParameterFile, QSettings::IniFormat);
    m_IniFile->setValue("sysParam/" + param.first, param.second);
    delete m_IniFile;
}

/**

* @brief Synonyms Judgement

*/
bool MainWindow::synonyms_check(std::string& str1, std::string& str2,
                                float tol, bool debug) {
    bool synonyms = false;
    // Tokenize the two given word groups
    std::vector<int64_t> input_1 = tokenize(string2wstring(str1), myVocab);
    std::vector<int64_t> input_2 = tokenize(string2wstring(str2), myVocab);
    // [batch_size, sequence_length]
    std::vector<int64_t> input_1_shape = {1, static_cast<int64_t>(input_1.size())};
    std::vector<int64_t> input_2_shape = {1, static_cast<int64_t>(input_2.size())};

    // Create the input tensor
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator,
                                                             OrtMemTypeCPU);
    Ort::Value input_1_tensor = Ort::Value::CreateTensor<int64_t>(memory_info,
                                                                  input_1.data(),
                                                                  input_1.size(),
                                                                  input_1_shape.data(),
                                                                  input_1_shape.size());
    Ort::Value input_2_tensor = Ort::Value::CreateTensor<int64_t>(memory_info,
                                                                  input_2.data(),
                                                                  input_2.size(),
                                                                  input_2_shape.data(),
                                                                  input_2_shape.size());

    // Input & Output Name of the Model
    std::vector<const char*> input_node_names = {"input_ids"};
    std::vector<const char*> output_node_names = {"output"};

    // Perform inference
    try {
        auto output_tensors = session->Run(Ort::RunOptions{nullptr},
                                           input_node_names.data(),
                                           &input_1_tensor,
                                           1, output_node_names.data(), 1);
        // Get the output result
        float* output_data_1 = output_tensors[0].GetTensorMutableData<float>();
        std::vector<float> output_vector_1(output_data_1,
                                           output_data_1 + 768*input_1_shape[0]*input_1_shape[1]);

       output_tensors = session->Run(Ort::RunOptions{nullptr},
                                     input_node_names.data(),
                                     &input_2_tensor,
                                     1, output_node_names.data(), 1);
        // Get the output result
        float* output_data_2 = output_tensors[0].GetTensorMutableData<float>();
        std::vector<float> output_vector_2(output_data_2,
                                           output_data_2 + 768*input_2_shape[0]*input_2_shape[1]);

        // Compute the Cos similarity
        float cos_similarity = cosine_similarity(output_vector_1, output_vector_2);
        if (debug) {
            qDebug() << "Cosine Similarity between" << QString::fromStdString(str1) << "and" << QString::fromStdString(str2) << ": ";
            qDebug() << cos_similarity;
        }
        return cos_similarity >= tol;
    }
    catch (const Ort::Exception& e) {
        qDebug() << "ONNX Runtime Error: " << e.what();
    }
    return synonyms;
}

/**

* @brief Testers for particular libraries
* @author Chris Chan
* @date 2024/10/20

*/
// Test for ONNX
void MainWindow::onnx_test() {
    std::string str1 = "小溪";
    std::string str2 = "溪流";
    qDebug() << QString::fromStdString(str1) << "和" << QString::fromStdString(str2) << "是近义词吗：" << synonyms_check(str1, str2, float(0.7), true);
}

/**

* @brief Slots for interacting with menubars and actions
* @author Chris Chan
* @date 2024/10/20

*/
void MainWindow::mode_switch(QAction* action) {
    QChar c = action->objectName().at(6);
    // Enable only the search function
    for(int i = 1; i <= 4; i++) {
        this->ui->tabWidget->setTabEnabled(i, false);
    }
    // Search Mode --- 0
    if(c == 'S') {
        sysParameterUpdate({"mode", "0"});
    }
    // IELTS Mode --- 1
    else if(c == 'I') {
        sysParameterUpdate({"mode", "1"});
        // Turn On all functions
        for(int i = 1; i <= 4; i++) {
            this->ui->tabWidget->setTabEnabled(i, true);
        }
    }
    // GRE Mode --- 2
    else {
        sysParameterUpdate({"mode", "2"});
        // Turn On read and write
        for(int i = 2; i <= 3; i++) {
            this->ui->tabWidget->setTabEnabled(i, true);
        }
    }
}
