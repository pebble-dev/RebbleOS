;; Make sure AC Clang is in emacs load path
(require 'auto-complete-clang)

(defun my-ac-cc-mode-setup ()
  (setq ac-sources (append '(ac-source-clang) ac-sources))
  (setq ac-clang-flags
        (mapcar(lambda (item)(concat (concat"-I" (file-name-directory (or load-file-name (buffer-file-name)))) item))
               (split-string
                "config
hardware
FreeRTOS/include
FreeRTOS/portable/GCC/ARM_CM4F
Libraries/CMSIS/Device/ST/STM32F4xx/Include
Libraries/STM32F4xx_StdPeriph_Driver/inc"))))

(add-hook 'c-mode-common-hook 'my-ac-cc-mode-setup)

(provide 'ac-clang-config)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Add the following to your .emacs file.
;; (add-to-list 'load-path "path-to-this-file")
;; (require 'ac-clang-config)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
