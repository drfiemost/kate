#ifndef COMMANDMODEL_H
#define COMMANDMODEL_H

#include <QAbstractTableModel>
#include <QVector>

class QAction;

class CommandModel : public QAbstractTableModel
{
    struct Item
    {
        QAction* action;
        int score;
    };

    Q_OBJECT
public:
    CommandModel(QObject* parent = nullptr);

    enum Role { Score = Qt::UserRole + 1 };

    void refresh(QList<QAction*> actions);

    int rowCount(const QModelIndex & parent = QModelIndex()) const override
    {
        if (parent.isValid()) {
            return 0;
        }
        return m_rows.size();
    }

    int columnCount(const QModelIndex & parent = QModelIndex()) const override
    {
        Q_UNUSED(parent);
        return 2;
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role) override
    {
        if (!index.isValid())
            return false;
        if (role == Role::Score) {
            auto row = index.row();
            m_rows[row].score = value.toInt();
        }
        return QAbstractTableModel::setData(index, value, role);
    }

    QVariant data(const QModelIndex &index, int role) const override;

private:
    QVector<Item> m_rows;

};

#endif // COMMANDMODEL_H
