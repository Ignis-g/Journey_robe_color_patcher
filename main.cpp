#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QMessageBox>
#include <QSettings>
#include <QFile>
#include <QDataStream>
#include <QPixmap>
#include <QIcon>
#include <QDir>

class JourneyColorPatcher : public QWidget {
    Q_OBJECT

public:
    JourneyColorPatcher(QWidget* parent = nullptr) : QWidget(parent) {
        setWindowTitle("Journey robe color patcher");
        setFixedSize(340, 120);

        offsets = { 0x161D7F, 0x2169F7 };
        values = { 1, 2, 3 };

        binaryFilePath = getBinaryFilePath();

        QHBoxLayout* mainLayout = new QHBoxLayout(this);

        for (int i : values) {
            QVBoxLayout* vbox = new QVBoxLayout;
            QLabel* imgLabel = new QLabel;
            QPixmap pix(QString("tier_%1.png").arg(i + 1));
            if (pix.isNull()) {
                QMessageBox::warning(this, "Warning", QString("Could not load tier_%1.png").arg(i + 1));
            }
            imgLabel->setPixmap(pix);
            thumbnails.append(pix);

            QPushButton* btn = new QPushButton(QString("Tier %1").arg(i + 1));
            connect(btn, &QPushButton::clicked, this, [=]() {
                confirmAndWrite(i);
            });

            imgLabel->setAlignment(Qt::AlignCenter);
            vbox->addWidget(imgLabel);
            vbox->addWidget(btn);
            mainLayout->addLayout(vbox);
        }

        QFrame* separator = new QFrame;
        separator->setFrameShape(QFrame::VLine);
        separator->setFrameShadow(QFrame::Sunken);
        mainLayout->addWidget(separator);

        currentLabel = new QLabel("Current");
        mainLayout->addWidget(currentLabel);

        updateCurrentValueLabel(offsets[0]);
    }

private:
    QList<int> offsets;
    QList<int> values;
    QList<QPixmap> thumbnails;
    QLabel* currentLabel;
    QString binaryFilePath;

    QString getBinaryFilePath() {
        QSettings settings("HKEY_CURRENT_USER\\Software\\Valve\\Steam", QSettings::NativeFormat);
        QString steamPath = settings.value("SteamPath").toString();
        if (steamPath.isEmpty()) {
            QMessageBox::critical(this, "Error", "Steam not found in registry.");
            exit(1);
        }
        QString path = steamPath + "/SteamApps/common/Journey/Journey.exe";
        if (!QFile::exists(path)) {
            QMessageBox::critical(this, "Error", "Journey.exe not found at expected location:\n" + path);
            exit(1);
        }
        return path;
    }

    void writeToFile(const QList<int>& offsets, int value) {
        QFile file(binaryFilePath);
        if (!file.open(QIODevice::ReadWrite)) {
            QMessageBox::critical(this, "Error", "Failed to open Journey.exe");
            return;
        }
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream << static_cast<quint32>(value);

        for (int offset : offsets) {
            file.seek(offset);
            file.write(data);
        }
        file.close();
    }

    int readValueAtOffset(int offset) {
        QFile file(binaryFilePath);
        if (!file.open(QIODevice::ReadOnly)) return 0;
        file.seek(offset);
        QByteArray data = file.read(4);
        QDataStream stream(data);
        stream.setByteOrder(QDataStream::LittleEndian);
        quint32 value;
        stream >> value;
        return value;
    }

    void confirmAndWrite(int value) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirmation",
            QString("Do you want to set Tier %1?").arg(value + 1),
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            writeToFile(offsets, value);
            updateCurrentValueLabel(offsets[0]);
        }
    }

    void updateCurrentValueLabel(int offset) {
        int value = readValueAtOffset(offset);
        if (value >= 1 && value - 1 < thumbnails.size()) {
            currentLabel->setPixmap(thumbnails[value - 1]);
        }
    }
};

#include "main.moc"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    JourneyColorPatcher patcher;
    patcher.show();
    return app.exec();
}
