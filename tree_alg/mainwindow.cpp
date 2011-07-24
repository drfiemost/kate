#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QtCore/qdebug.h>
#include "QTime"
#include <time.h>

QString MainWindow::defaultHistoryFile = NULL;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    histLimit(0),
    isTesting(false)
{
    ui->setupUi(this);
    ui->textEdit->setReadOnly(true);
    ui->textEdit_2->setReadOnly(true);
    ui->textEdit_3->setReadOnly(true);
    ui->textEdit->setVisible(false);
    ui->textEdit_2->setVisible(false);
    ui->textEdit_3->setVisible(true);
    ui->label_6->setVisible(false);
    folding_tree = new FoldingTree();
    ui->textEdit_3->setText(folding_tree->toString());
    ui->spinBox_3->setMinimum(1);
    ui->spinBox_2->setMaximum(0);
    ui->spinBox_3->setMaximum(0);
    ui->spinBox_4->setMaximum(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// insert Start Node
void MainWindow::on_pushButton_2_released()
{
  int position = ui->spinBox_2->value();
  QString histString(QString("***********\n(%1)insert start node at position %2 :\n").arg(FoldingTree::nOps++).arg(position));
  folding_tree->insertStartNode(position);
  ui->textEdit->selectAll();
  ui->textEdit->clear();
  QString string = folding_tree->toString();
  histString.append(string);
  histString.append("\n***********\n\n");
  FoldingTree::history.append(histString);
  FoldingTree::movesHistory.append(QString("insertStart %1\n").arg(position));
  if (histLimit) {
    while (FoldingTree::history.size() > histLimit)
      FoldingTree::history.pop_front();
  }
  if (ui->checkBox_4->isChecked()) {
    saveHistoryToFile();
  }
  ui->spinBox_2->setMaximum(folding_tree->nodeMap.size() - 1);
  ui->spinBox_3->setMaximum(folding_tree->nodeMap.size() - 1);
  ui->spinBox_4->setMaximum(folding_tree->nodeMap.size() - 1);
  ui->label_6->setVisible(false);
  ui->textEdit->setVisible(false);
  ui->textEdit_2->setVisible(false);
  ui->textEdit_3->setVisible(true);
  ui->textEdit_3->setPlainText(string);
  check();
}


// Delete node
void MainWindow::on_pushButton_3_released()
{
  int position = ui->spinBox_3->value();
  QString histString(QString("***********\n(%1)delete node from position %2 :\n").arg(FoldingTree::nOps++).arg(position));
  folding_tree->deleteNode(position);
  ui->textEdit->selectAll();
  ui->textEdit->clear();
  QString string = folding_tree->toString();
  histString.append(string);
  histString.append("\n***********\n\n");
  FoldingTree::history.append(histString);
  FoldingTree::movesHistory.append(QString("delete %1\n").arg(position));
  if (histLimit) {
    while (FoldingTree::history.size() > histLimit)
      FoldingTree::history.pop_front();
  }
  if (ui->checkBox_4->isChecked()) {
    saveHistoryToFile();
  }
  ui->spinBox_2->setMaximum(folding_tree->nodeMap.size() - 1);
  ui->spinBox_3->setMaximum(folding_tree->nodeMap.size() - 1);
  ui->spinBox_4->setMaximum(folding_tree->nodeMap.size() - 1);
  ui->label_6->setVisible(false);
  ui->textEdit->setVisible(false);
  ui->textEdit_2->setVisible(false);
  ui->textEdit_3->setVisible(true);
  ui->textEdit_3->setPlainText(string);
  check();
}


// insert End node
void MainWindow::on_pushButton_4_released()
{
  int position = ui->spinBox_4->value();
  QString histString(QString("***********\n(%1)insert end node at position %2 :\n").arg(FoldingTree::nOps++).arg(position));
  folding_tree->insertEndNode(position);
  ui->textEdit->selectAll();
  ui->textEdit->clear();
  QString string = folding_tree->toString();
  histString.append(string);
  histString.append("\n***********\n\n");
  FoldingTree::history.append(histString);
  FoldingTree::movesHistory.append(QString("insertEnd %1\n").arg(position));
  if (histLimit) {
    while (FoldingTree::history.size() > histLimit)
      FoldingTree::history.pop_front();
  }
  if (ui->checkBox_4->isChecked()) {
    saveHistoryToFile();
  }
  ui->spinBox_2->setMaximum(folding_tree->nodeMap.size() - 1);
  ui->spinBox_3->setMaximum(folding_tree->nodeMap.size() - 1);
  ui->spinBox_4->setMaximum(folding_tree->nodeMap.size() - 1);
  ui->label_6->setVisible(false);
  ui->textEdit->setVisible(false);
  ui->textEdit_2->setVisible(false);
  ui->textEdit_3->setVisible(true);
  ui->textEdit_3->setPlainText(string);
  check();
}

// driver method
void MainWindow::on_pushButton_5_released()
{
  //testMergeDriver();
}

// print method
void MainWindow::on_pushButton_6_released()
{
  ui->textEdit->selectAll();
  ui->textEdit->clear();
  QString string = folding_tree->toString();
  ui->label_6->setVisible(false);
  ui->textEdit->setVisible(false);
  ui->textEdit_2->setVisible(false);
  ui->textEdit_3->setVisible(true);
  ui->textEdit_3->setPlainText(string);
}

// Start of Testing Methods
/*
void MainWindow::testMergeDriver()
{
  QVector <FoldingNode *> list1;
  list1.append(new FoldingNode(1,1,NULL));
  list1.append(new FoldingNode(3,1,NULL));
  list1.append(new FoldingNode(6,1,NULL));
  list1.append(new FoldingNode(9,1,NULL));

  QVector <FoldingNode *> list2;
  list2.append(new FoldingNode(2,1,NULL));
  list2.append(new FoldingNode(4,1,NULL));
  list2.append(new FoldingNode(5,1,NULL));
  list2.append(new FoldingNode(10,1,NULL));

  qDebug()<<QString("new output:");
  folding_tree->mergeChildren(list1,list2);
  for (int i = 0 ; i < list1.size() ; ++ i) {
    qDebug()<<QString("%1").arg(list1[i]->nodeNo);
  }
  qDebug()<<QString("*****");
}*/

void MainWindow::on_checkBox_stateChanged(int newState)
{
    if (newState)
      FoldingTree::displayDetails = true;
    else
      FoldingTree::displayDetails = false;
}

void MainWindow::on_checkBox_2_stateChanged(int newState)
{
  if (newState)
    FoldingTree::displayChildrenDetails = true;
  else
    FoldingTree::displayChildrenDetails = false;
}

void MainWindow::on_checkBox_3_stateChanged(int newState)
{
  if (newState)
    FoldingTree::displayDuplicates = true;
  else
    FoldingTree::displayDuplicates = false;
}

// Open FileDialog to "Save As" the history
void MainWindow::on_pushButton_7_released()
{
  QString fileName = QFileDialog::getSaveFileName();
  saveHistoryToFile(fileName);
}

// Sets history size (limit)
void MainWindow::on_spinBox_valueChanged(int newValue)
{
  histLimit = newValue;
}

// Clears History
void MainWindow::on_pushButton_8_released()
{
  FoldingTree::history.clear();
}

// Prints the history to fileName
void MainWindow::saveHistoryToFile(QString fileName)
{
  QFile file(fileName);
  file.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream out(&file);
  foreach (QString hist, FoldingTree::history) {
    out<<hist;
  }
  file.close();

  QFile file2(fileName+"_moves");
  file2.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream out2(&file2);
  foreach (QString hist, FoldingTree::movesHistory) {
    out2<<hist;
  }
  file2.close();
}

// Choose file to save automatic history
void MainWindow::on_checkBox_4_stateChanged(int newState)
{
  if (newState > 0) {
    defaultHistoryFile = QFileDialog::getSaveFileName(0,"Select file for history");
    if (defaultHistoryFile.isNull()) {
      ui->checkBox_4->setChecked(false);
      ui->checkBox_4->setCheckState(Qt::Unchecked);
    }
  }
}

void MainWindow::displayTree()
{
  ui->textEdit->selectAll();
  ui->textEdit->clear();
  folding_tree->buildTreeString(folding_tree->root,1);
  ui->textEdit->setVisible(true);
  ui->textEdit_3->setVisible(false);
  ui->textEdit->setPlainText(folding_tree->treeString);
}

void MainWindow::displayStack()
{
  ui->textEdit_2->selectAll();
  ui->textEdit_2->clear();
  folding_tree->buildStackString();
  ui->label_6->setVisible(true);
  ui->textEdit_2->setVisible(true);
  ui->textEdit_3->setVisible(false);
  ui->textEdit_2->setPlainText(folding_tree->stackString);
}

// Check button
// Prints the output of the stack alg
void MainWindow::on_pushButton_released()
{
  displayTree();
  displayStack();
  QMessageBox msgBox;
  if (folding_tree->isCorrect())
    msgBox.setText("OK!!!");
  else
    msgBox.setText("ERROR!!!");
  msgBox.exec();
}

// Prints the output of the tree alg
// with the same format as the stack alg
void MainWindow::on_pushButton_9_released()
{
  displayTree();
}

void MainWindow::check()
{
  if (ui->checkBox_5->isChecked() == false)
    return;
  folding_tree->buildStackString();
  folding_tree->buildTreeString(folding_tree->root,1);
  if (folding_tree->isCorrect())
    return;

  displayTree();
  displayStack();
  QMessageBox msgBox;
  msgBox.setText("ERROR!!!");
  msgBox.exec();
}

void MainWindow::on_pushButton_10_released()
{
  QString inputFileName = QFileDialog::getSaveFileName(0,"Select file for Automatic Input");
  if (inputFileName.isNull() == false) {
    automaticTestingFromFile(inputFileName);
  }
}

void MainWindow::automaticTestingFromFile(QString fileName)
{
  QFile file(fileName);
  if (!file.open (QIODevice::ReadOnly))
    return;
  QTextStream stream (&file);
  QString line;
  while( !stream.atEnd() ) {
    line = stream.readLine();
    QStringList instrList = line.split(" ");
    if (instrList[0] == "delete") {
      QString test = instrList[1];
      deleteNode(test.toInt());
    }
    else if (instrList[0] == "insertStart") {
      QString test = instrList[1];
      insertStart(test.toInt());
    }
    else if (instrList[0] == "insertEnd") {
      QString test = instrList[1];
      insertEnd(test.toInt());
    }
  }
  file.close();
}

void MainWindow::automaticTesting()
{
  qsrand(time(NULL));
  for (int i = 0 ; i < ui->spinBox_5->value() && isTesting && folding_tree->isCorrect() ; ++i) {
    int move = qrand() % 3;
    int pos = qrand() % folding_tree->nodeMap.size();
    qDebug()<<QString("new move: %1, %2").arg(move).arg(pos);
    switch (move) {
    case 0 : insertStart(pos);
      break;
    case 1 : insertEnd(pos);
      break;
    default : deleteNode(pos);
          }
  }
  ui->pushButton_11->setText("Start Test");
  isTesting = false;
}

void MainWindow::insertStart(int position)
{
  ui->spinBox_2->setValue(position);
  on_pushButton_2_released();
}

void MainWindow::insertEnd(int position)
{
  ui->spinBox_4->setValue(position);
  on_pushButton_4_released();
}

void MainWindow::deleteNode(int position)
{
  ui->spinBox_3->setValue(position);
  on_pushButton_3_released();
}

void MainWindow::on_pushButton_11_released()
{
  if (isTesting) {
    ui->pushButton_11->setText("Start Test");
    isTesting = false;
  }
  else {
    ui->pushButton_11->setText("Stop Test");
    isTesting = true;
    automaticTesting();
  }
}
