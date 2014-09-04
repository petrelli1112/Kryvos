/**
 * Kryvos File Encryptor - Encrypts and decrypts files.
 * Copyright (C) 2014 Andrew Dolby
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact : andrewdolby@gmail.com
 */

#ifndef KRYVOS_GUI_MESSAGEFRAME_HPP_
#define KRYVOS_GUI_MESSAGEFRAME_HPP_

#include <QtWidgets/QFrame>
#include <memory>

/*!
 * \brief The MessageFrame class contains a text edit display used to inform the
 * user of the status of encryption and decryption operations.
 */
class MessageFrame : public QFrame {
  Q_OBJECT

 public:
  /*!
   * \brief MessageFrame Constructs a message frame, which is used to inform a
   * user about the status of encryption and decryption operations.
   * \param parent The QWidget parent of this message frame.
   */
  explicit MessageFrame(QWidget* parent = nullptr);

  /*!
   * \brief ~MessageFrame Destroys a message frame.
   */
  virtual ~MessageFrame();

  /*!
   * \brief appendPlainText Appends text to the message frame's text edit.
   * \param message
   */
  void appendPlainText(const QString& message);

 private:
  class MessageFramePrivate;
  std::unique_ptr<MessageFramePrivate> pimpl;
};

#endif // KRYVOS_GUI_MESSAGEFRAME_HPP_