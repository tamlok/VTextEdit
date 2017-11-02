#ifndef VTEXTDOCUMENTLAYOUT_H
#define VTEXTDOCUMENTLAYOUT_H

#include <QAbstractTextDocumentLayout>
#include <QVector>

class VTextDocumentLayout : public QAbstractTextDocumentLayout
{
    Q_OBJECT

public:
    explicit VTextDocumentLayout(QTextDocument *p_doc);

    void draw(QPainter *p_painter, const PaintContext &p_context) Q_DECL_OVERRIDE;

    int hitTest(const QPointF &p_point, Qt::HitTestAccuracy p_accuracy) const Q_DECL_OVERRIDE;

    int pageCount() const Q_DECL_OVERRIDE;

    QSizeF documentSize() const Q_DECL_OVERRIDE;

    QRectF frameBoundingRect(QTextFrame *p_frame) const Q_DECL_OVERRIDE;

    QRectF blockBoundingRect(const QTextBlock &p_block) const Q_DECL_OVERRIDE;

    void setCursorWidth(int p_width);

    int cursorWidth() const;

protected:
    void documentChanged(int p_from, int p_charsRemoved, int p_charsAdded) Q_DECL_OVERRIDE;

private:
    struct BlockInfo
    {
        BlockInfo()
        {
            reset();
        }

        void reset()
        {
            m_offset = -1;
            m_rect = QRectF();
        }

        bool hasOffset() const
        {
            return m_offset > -1 && !m_rect.isNull();
        }

        qreal top() const
        {
            Q_ASSERT(hasOffset());
            return m_offset;
        }

        qreal bottom() const
        {
            Q_ASSERT(hasOffset());
            return m_offset + m_rect.height();
        }

        // Y offset of this block.
        // -1 for invalid.
        qreal m_offset;

        // The bounding rect of this block, including the margins.
        // Null for invalid.
        QRectF m_rect;
    };

    void layoutBlock(const QTextBlock &p_block);

    // Clear the layout of @p_block.
    // Also clear all the offset behind this block.
    void clearBlockLayout(QTextBlock &p_block);

    // Clear the offset of all the blocks from @p_blockNumber.
    void clearOffsetFrom(int p_blockNumber);

    // Fill the offset filed from @p_blockNumber + 1.
    void fillOffsetFrom(int p_blockNumber);

    // Update block count to @p_count due to document change.
    // Maintain m_blocks.
    // @p_changeStartBlock is the block number of the start block in this change.
    void updateBlockCount(int p_count, int p_changeStartBlock);

    bool validateBlocks() const;

    // Set the width of the page.
    void setPageWidth(int p_width);

    void finishBlockLayout(const QTextBlock &p_block);

    int previousValidBlockNumber(int p_number) const;

    // Update block count and m_blocks size.
    void updateDocumentSize();

    QVector<QTextLayout::FormatRange> formatRangeFromSelection(const QTextBlock &p_block,
                                                               const QVector<Selection> &p_selections) const;

    // Get the block range [first, last] by rect @p_rect.
    // @p_rect: a clip region in document coordinates. If null, returns all the blocks.
    // Return [-1, -1] if no valid block range found.
    void blockRangeFromRect(const QRectF &p_rect, int &p_first, int &p_last) const;

    // Return a rect from the layout.
    // Return a null rect if @p_block has not been layouted.
    QRectF blockRectFromTextLayout(const QTextBlock &p_block);

    // Return the width of the block regarding to document.
    inline qreal blockWidthInDocument(int p_width) const;

    // Update document size when only block @p_blockNumber is changed and the height
    // remain the same.
    void updateDocumentSizeWithOneBlockChanged(int p_blockNumber);

    // Available width of the page.
    qreal m_pageWidth;

    // Document margin on left/right/bottom.
    qreal m_margin;

    // Maximum width of the contents.
    qreal m_width;

    // The block number of the block which contains the m_width.
    int m_maximumWidthBlockNumber;

    // Height of all the document (all the blocks).
    qreal m_height;

    // Block count of the document.
    int m_blockCount;

    // Width of the cursor.
    int m_cursorWidth;

    QVector<BlockInfo> m_blocks;
};

inline void VTextDocumentLayout::setPageWidth(int p_width)
{
    m_pageWidth = m_width = p_width;
}

inline qreal VTextDocumentLayout::blockWidthInDocument(int p_width) const
{
    return p_width + 10;
}

#endif // VTEXTDOCUMENTLAYOUT_H
